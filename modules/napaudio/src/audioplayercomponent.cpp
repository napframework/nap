#include "audioplayercomponent.h"

// Nap includes
#include <nap/entity.h>
#include <nap/logger.h>

// Audio includes
#include "utility/audiofilereader.h"

// RTTI
RTTI_BEGIN_CLASS(nap::audio::AudioPlayerComponent)
    RTTI_PROPERTY("AudioInterface",	&nap::audio::AudioPlayerComponent::mAudioInterface, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("AudioFile", &nap::audio::AudioPlayerComponent::mAudioFilePath, nap::rtti::EPropertyMetaData::FileLink)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioPlayerComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&)
RTTI_END_CLASS


namespace nap {
    
    namespace audio {
        
        bool AudioPlayerComponentInstance::init(const ObjectPtr<Component>& resourcePtr, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
        {
            AudioPlayerComponent* resource = rtti_cast<AudioPlayerComponent>(resourcePtr.get());
            
            // Read an audio file
            if (!nap::audio::readAudioFile(resource->mAudioFilePath, audioFileBuffer, fileSampleRate))
            {
                errorState.fail("Unable to load audio file: " + resource->mAudioFilePath);
                return false;
            }
            
            int channelCount = std::min<int>(audioFileBuffer.getChannelCount(), resource->mAudioInterface->mOutputChannelCount);
            for (auto i = 0; i < channelCount; ++i)
            {
                mPlayers.emplace_back(std::make_unique<BufferPlayer>(resource->mAudioInterface->getNodeManager()));
                mOutputs.emplace_back(std::make_unique<AudioOutputNode>(resource->mAudioInterface->getNodeManager()));
                mOutputs[i]->outputChannel = i;
                mOutputs[i]->audioInput.connect(mPlayers[i]->audioOutput);
                mPlayers[i]->play(&audioFileBuffer[i], 0, fileSampleRate / resource->mAudioInterface->mSampleRate);                
            }
            
            return true;
        }
        
        
    }
    
}
