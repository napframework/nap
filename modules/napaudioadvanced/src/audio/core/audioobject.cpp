#include "audioobject.h"


// Nap includes
#include <entity.h>
#include <nap/core.h>

// Audio includes
#include <audio/service/audioservice.h>


// RTTI
RTTI_DEFINE_BASE(nap::audio::AudioObject)
RTTI_DEFINE_BASE(nap::audio::AudioObjectInstance)
RTTI_DEFINE_BASE(nap::audio::MultiChannelObject)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::MultiChannelObjectInstance)
    RTTI_FUNCTION("getChannel", &nap::audio::MultiChannelObjectInstance::getChannel)
RTTI_END_CLASS


namespace nap
{
    
    namespace audio
    {
        
        
        std::unique_ptr<AudioObjectInstance> AudioObject::instantiate(AudioService& service, utility::ErrorState& errorState)
        {
            auto instance = createInstance();
            mInstance = instance.get();
            if (errorState.check(instance->init(service, errorState), "Failed to instantiate object %s", mID.c_str()))
                return instance;
            else
                return nullptr;
        }

        
        std::unique_ptr<AudioObjectInstance> MultiChannelObject::createInstance()
        {
            return std::make_unique<MultiChannelObjectInstance>(*this);            
        }
        
        
        bool MultiChannelObjectInstance::init(AudioService& service, utility::ErrorState& errorState)
        {
            auto resource = rtti_cast<MultiChannelObject>(&getResource());
            for (auto channel = 0; channel < resource->getChannelCount(); ++channel)
            {
                auto node = resource->createNode(channel, service);
                assert(node->getOutputs().size() == 1);
                mNodes.emplace_back(std::move(node));
            }
            return true;
        }

        
        Node* MultiChannelObjectInstance::getChannel(int channel)
        {
            if (channel < mNodes.size())
                return mNodes[channel].getRaw();
            else
                return nullptr;
        }


    
    }
    
}
