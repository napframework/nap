#include "multiobject.h"

// Nap includes
#include <entity.h>
#include <nap/logger.h>

// Audio includes
#include <audio/core/audioobject.h>


// RTTI
RTTI_BEGIN_CLASS(nap::audio::MultiObject)
    RTTI_PROPERTY("Object", &nap::audio::MultiObject::mObject, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("InstanceCount", &nap::audio::MultiObject::mInstanceCount, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("IsActive", &nap::audio::MultiObject::mIsActive, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::audio::MultiObjectInstance)
    RTTI_FUNCTION("getObject", &nap::audio::MultiObjectInstance::getObjectNonTyped)
    RTTI_FUNCTION("getObjectCount", &nap::audio::MultiObjectInstance::getObjectCount)
    RTTI_FUNCTION("setActive", &nap::audio::MultiObjectInstance::setActive)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        

        std::unique_ptr<AudioObjectInstance> MultiObject::createInstance(NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            auto instance = std::make_unique<MultiObjectInstance>();
            if (!instance->init(*mObject, mInstanceCount, mIsActive, nodeManager, errorState))
            {
                errorState.fail("Failed to initialize %s", mID.c_str());
                return nullptr;
            }
            return std::move(instance);
        }
        
        
        bool MultiObjectInstance::init(AudioObject& objectResource, int instanceCount, bool isActive, NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            mNodeManager = &nodeManager;
            
            // Instantiate the objects.
            mObjectResource = &objectResource;
            
            for (auto i = 0; i < instanceCount; ++i)
            {
                auto instance = mObjectResource->instantiate<AudioObjectInstance>(nodeManager, errorState);
                if (instance == nullptr)
                    return false;
                mObjects.emplace_back(std::move(instance));
            }
            
            // Determine the channel count
            int channelCount = 0;
            if (mObjects.empty())
            {
                // Create a dummy object instance to acquire the channel count..
                auto instance = mObjectResource->instantiate<AudioObjectInstance>(nodeManager, errorState);
                if (instance == nullptr)
                    return false;
                channelCount = instance->getChannelCount();
            }
            else
                channelCount = mObjects[0]->getChannelCount();
            
            // Create the output mixers
            for (auto channel = 0; channel < channelCount; ++channel)
            {
                // we dont connect anything yet
                mMixers.emplace_back(nodeManager.makeSafe<MixNode>(nodeManager));
            }
            
            // Connect the objects that are active
            for (auto i = 0; i < mObjects.size(); ++i)
            {
                if (isActive)
                {
                    auto object = mObjects[i].get();
                    for (auto channel = 0; channel < mMixers.size(); ++channel)
                        mMixers[channel]->inputs.connect(*object->getOutputForChannel(channel));
                }
            }

            return true;
        }

        
        AudioObjectInstance* MultiObjectInstance::getObjectNonTyped(unsigned int index)
        {
            if (index < mObjects.size())
                return mObjects[index].get();
            
            nap::Logger::warn("MultiObjectInstance %s: index for getObject() out of bounds", getName().c_str());
            return nullptr;
        }
        
        
        AudioObjectInstance* MultiObjectInstance::addObjectNonTyped(utility::ErrorState& errorState)
        {
            if (mObjectResource == nullptr)
                return nullptr;
            auto instance = mObjectResource->instantiate<AudioObjectInstance>(*mNodeManager, errorState);
            if (instance == nullptr)
                return nullptr;
            auto result = instance.get();
            mObjects.emplace_back(std::move(instance));
            return result;
        }
        
        
        bool MultiObjectInstance::setActive(AudioObjectInstance* object, bool isActive)
        {
            for (auto& element : mObjects)
                if (element.get() == object)
                {
                    if (isActive)
                        for (auto channel = 0; channel < mMixers.size(); ++channel)
                            mMixers[channel]->inputs.enqueueConnect(*object->getOutputForChannel(channel));
                    else
                        for (auto channel = 0; channel < mMixers.size(); ++channel)
                            mMixers[channel]->inputs.enqueueDisconnect(*object->getOutputForChannel(channel));
                    
                    return true;
                }
            return false;
        }

        
        
        OutputPin* MultiObjectInstance::getOutputForChannel(int channel)
        {
            return &mMixers[channel]->audioOutput;
        }
        
        
        int MultiObjectInstance::getChannelCount() const
        {
            return mMixers.size();
        }
        
        
        int MultiObjectInstance::getInputChannelCount() const
        {
            if (mObjects.empty())
                return 0;
            return mObjects[0]->getInputChannelCount();
        }



        void MultiObjectInstance::connect(unsigned int channel, OutputPin& pin)
        {
            for (auto& object : mObjects)
                object->tryConnect(channel, pin);
        }
        
        
        void MultiObjectInstance::connect(MultiObjectInstance& inputMulti)
        {
            for (auto index = 0; index < getObjectCount(); ++index)
            {
                auto object = mObjects[index].get();
                auto inputObject = inputMulti.getObjectNonTyped(index % inputMulti.getObjectCount());
                object->connect(*inputObject);
            }
        }
        
        
    }
    
}
