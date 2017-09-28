#include "playercomponent.h"

// Nap includes
#include <nap/entity.h>


// RTTI
RTTI_BEGIN_CLASS(nap::audio::PlayerComponent)
    RTTI_PROPERTY("AudioBuffer", &nap::audio::PlayerComponent::mAudioBuffer, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Routing", &nap::audio::PlayerComponent::mChannelsToPlay, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Speed", &nap::audio::PlayerComponent::mSpeed, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::PlayerComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap {
    
    namespace audio {
        
        bool PlayerComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
        {
            PlayerComponent* resource = rtti_cast<PlayerComponent>(getComponent());

            auto& nodeManager = getNodeManager();
            
            for (auto i = 0; i < resource->mChannelsToPlay.size(); ++i)
            {
                if (resource->mChannelsToPlay[i] >= resource->mAudioBuffer->getChannelCount())
                {
                    errorState.fail("Trying to play channel out of bounds of buffer");
                    return false;
                }
                
                mPlayers.emplace_back(std::make_unique<BufferPlayerNode>(nodeManager));
                mPlayers[i]->play(resource->mAudioBuffer->getBuffer()[resource->mChannelsToPlay[i]], 0, resource->mSpeed * resource->mAudioBuffer->getSampleRate() / nodeManager.getSampleRate());
            }
            
            return true;
        }
        
        
    }
        
}
