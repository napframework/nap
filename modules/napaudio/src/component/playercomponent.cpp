#include "playercomponent.h"

// Nap includes
#include <nap/entity.h>


// RTTI
RTTI_BEGIN_CLASS(nap::audio::PlayerComponent)
    RTTI_PROPERTY("AudioFile", &nap::audio::PlayerComponent::mAudioBuffer, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::PlayerComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap {
    
    namespace audio {
        
        bool PlayerComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
        {
            PlayerComponent* resource = rtti_cast<PlayerComponent>(getComponent());

            for (auto i = 0; i < resource->mAudioBuffer->getBuffer().getChannelCount(); ++i)
            {
                mPlayers.emplace_back(std::make_unique<BufferPlayer>(resource->mAudioInterface->getNodeManager()));
                mPlayers[i]->play(resource->mAudioBuffer->getBuffer()[i], 0, resource->mAudioBuffer->getSampleRate() / resource->mAudioInterface->mSampleRate);
            }
            
            return true;
        }
        
        
    }
        
}
