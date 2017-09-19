#include "outputcomponent.h"

// Nap includes
#include <nap/entity.h>
#include <nap/core.h>

// Audio includes
#include <service/audioservice.h>
#include "audiocomponent.h"

// RTTI
RTTI_BEGIN_CLASS(nap::audio::OutputComponent)
    RTTI_PROPERTY("Input", &nap::audio::OutputComponent::mInput, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Routing", &nap::audio::OutputComponent::mChannelRouting, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::OutputComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap {
    
    namespace audio {
    
        bool audio::OutputComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
        {
            OutputComponent* resource = rtti_cast<OutputComponent>(getComponent());
            
            AudioComponentInstance* input = resource->mInput.get();
            auto& nodeManager = getEntityInstance()->getCore()->getService<AudioService>()->getNodeManager();
            
            auto channelCount = resource->mChannelRouting.size();
            for (auto channel = 0; channel < channelCount; ++channel)
            {
                if (resource->mChannelRouting[channel] >= input->getChannelCount())
                {
                    errorState.fail("Trying to rout channel that is out of bounds.");
                    return false;
                }
                
                mOutputs.emplace_back(std::make_unique<OutputNode>(nodeManager, true));
                mOutputs[channel]->setOutputChannel(channel);
                mOutputs[channel]->audioInput.connect(input->getOutputForChannel(resource->mChannelRouting[channel]));
            }
            
            return true;
        }

    }
    
}
