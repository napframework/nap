#include "inputcomponent.h"

// Nap includes
#include <entity.h>
#include <nap/core.h>

// Audio includes
#include <audio/core/audionodemanager.h>
#include <audio/service/audioservice.h>

// RTTI
RTTI_BEGIN_CLASS(nap::audio::AudioInputComponent)
    RTTI_PROPERTY("Channels", &nap::audio::AudioInputComponent::mChannels, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioInputComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
    
        bool AudioInputComponentInstance::init(utility::ErrorState& errorState)
        {
            auto resource = getComponent<AudioInputComponent>();
            NodeManager* nodeManager = &getEntityInstance()->getCore()->getService<AudioService>(rtti::ETypeCheck::EXACT_MATCH)->getNodeManager();

            for (auto channel = 0; channel < resource->mChannels.size(); ++channel)
            {
                if (!errorState.check(resource->mChannels[channel] >= nodeManager->getInputChannelCount(), "Input channel out of bounds"))
                    return false;
                auto node = make_node<InputNode>(*nodeManager);
                node->setInputChannel(resource->mChannels[channel]);
                mInputNodes.emplace_back(std::move(node));
            }
            return true;
        }
        
    }
    
}
