#include "outputcomponent.h"

// Nap includes
#include <entity.h>
#include <nap/core.h>

// Audio includes
#include <audio/service/audioservice.h>
#include "audiocomponentbase.h"

// RTTI
RTTI_BEGIN_CLASS(nap::audio::OutputComponent)
    RTTI_PROPERTY("Input", &nap::audio::OutputComponent::mInput, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Routing", &nap::audio::OutputComponent::mChannelRouting, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::OutputComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
    
        bool audio::OutputComponentInstance::init(utility::ErrorState& errorState)
        {
            OutputComponent* resource = getComponent<OutputComponent>();
            
            auto& nodeManager = getEntityInstance()->getCore()->getService<AudioService>(rtti::ETypeCheck::IS_DERIVED_FROM)->getNodeManager();
            
            auto channelCount = resource->mChannelRouting.size();
            if (channelCount > mInput->getChannelCount())
            {
                errorState.fail("Trying to rout channel that is out of bounds of input.");
                return false;
            }
            
            for (auto channel = 0; channel < channelCount; ++channel)
            {
                int destinationChannel = resource->mChannelRouting[channel];
                if (destinationChannel < 0)
                    continue;
                auto node = std::make_unique<OutputNode>(nodeManager);
                node->audioInput.connect(mInput->getOutputForChannel(channel));
                node->setOutputChannel(destinationChannel);
                mOutputs.emplace_back(std::move(node));
            }
            
            return true;
        }

    }
    
}
