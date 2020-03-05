#include "chain.h"

RTTI_BEGIN_CLASS(nap::audio::Chain)
    RTTI_PROPERTY("Objects", &nap::audio::Chain::mObjects, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("Input", &nap::audio::Chain::mInput, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::ChainInstance)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        std::unique_ptr<AudioObjectInstance> Chain::createInstance(NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            if (mObjects.empty())
            {
                errorState.fail("Objects property in effect chain can not be empty");
                return nullptr;
            }
            
            auto instance = std::make_unique<ChainInstance>();
            for (auto& object : mObjects)
            {
                if (!instance->addObject(*object, nodeManager, errorState))
                {
                    errorState.fail("Failed to add object to chain: %s", object->mID.c_str());
                    return nullptr;
                }
            }
            
            if (mInput != nullptr)
                instance->AudioObjectInstance::connect(*mInput->getInstance());
            
            return std::move(instance);
        }
        
        
        bool ChainInstance::addObject(AudioObject& resource, NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            auto instance = instantiateObjectInChain(resource, nodeManager, errorState);
            if (instance == nullptr)
                return false;
            return addObject(std::move(instance), errorState);
        }
        
        
        bool ChainInstance::addObject(std::unique_ptr<AudioObjectInstance> object, utility::ErrorState& errorState)
        {
            if (!mObjects.empty())
            {
                auto previous = mObjects.back().get();
                if (!connectObjectsInChain(*previous, *object, errorState))
                    return false;
            }
            mObjects.emplace_back(std::move(object));
            return true;
        }

        
        
        AudioObjectInstance* ChainInstance::getObjectNonTyped(unsigned int index)
        {
            if (index < mObjects.size())
                return mObjects[index].get();
            else
                return nullptr;
        }
        
        
        bool ChainInstance::connectObjectsInChain(AudioObjectInstance& source, AudioObjectInstance& destination, utility::ErrorState& errorState)
        {
            auto destinationInput = dynamic_cast<IMultiChannelInput*>(&destination);
            if (destinationInput == nullptr)
            {
                errorState.fail("Object in chain has no input: %s", destination.getName().c_str());
                return false;
            }
            destinationInput->connect(source);
            return true;
        }


        std::unique_ptr<AudioObjectInstance> ChainInstance::instantiateObjectInChain(AudioObject& resource, NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            auto newObject = resource.instantiate<AudioObjectInstance>(nodeManager, errorState);
            if (newObject == nullptr)
            {
                errorState.fail("Failed to create object in chain: %s", resource.mID.c_str());
                return nullptr;
            }
            return newObject;
        }
        
        
        
    }
    
}
