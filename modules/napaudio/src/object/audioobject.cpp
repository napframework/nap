#include "audioobject.h"


// Nap includes
#include <nap/entity.h>
#include <nap/core.h>

// Audio includes
#include <service/audioservice.h>


// RTTI
RTTI_BEGIN_CLASS(nap::audio::AudioObject)
RTTI_END_CLASS


namespace nap {
    
    namespace audio {
        
        
        std::unique_ptr<AudioObjectInstance> AudioObject::instantiate(utility::ErrorState& errorState)
        {
            auto instance = createInstance();
            mInstance = instance.get();
            if (!instance->init(errorState))
                return nullptr;
            else
                return instance;
        }

        
        std::unique_ptr<AudioObjectInstance> MultiChannelObject::createInstance()
        {
            return std::make_unique<MultiChannelObjectInstance>(*this);            
        }
        
        
        bool MultiChannelObjectInstance::init(utility::ErrorState& errorState)
        {
            auto resource = rtti_cast<MultiChannelObject>(&getResource());
            for (auto channel = 0; channel < resource->getChannelCount(); ++channel)
            {
                auto node = resource->createNode(channel);
                assert(node->getOutputs().size() == 1);
                mNodes.emplace_back(std::move(node));
            }
            return true;
        }

    
    }
    
}
