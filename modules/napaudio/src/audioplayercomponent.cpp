#include "audioplayercomponent.h"

// Nap includes
#include <nap/entity.h>
#include <nap/logger.h>

// Audio includes
#include "utility/audiofilereader.h"

// RTTI
RTTI_BEGIN_CLASS(nap::audio::AudioPlayerComponent)
    RTTI_PROPERTY("AudioInterface",	&nap::audio::AudioPlayerComponent::mAudioInterface, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("AudioFile", &nap::audio::AudioPlayerComponent::mAudioFile, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Panning", &nap::audio::AudioPlayerComponent::mPanning, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Gain", &nap::audio::AudioPlayerComponent::mGain, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioPlayerComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&)
RTTI_END_CLASS


namespace nap {
    
    namespace audio {
        
        /**
         * Initializes the audio node system to play back an audio buffer, either mono or stereo
         */
        bool AudioPlayerComponentInstance::init(const ObjectPtr<Component>& resourcePtr, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
        {
            AudioPlayerComponent* resource = rtti_cast<AudioPlayerComponent>(resourcePtr.get());
            
            // Mono mode
            if (resource->mAudioFile->getBuffer().getChannelCount() == 1)
            {
                // one player for the mono buffer
                mPlayers.emplace_back(std::make_unique<BufferPlayer>(resource->mAudioInterface->getNodeManager()));
                mPlayers[0]->play(resource->mAudioFile->getBuffer()[0], 0, resource->mAudioFile->getSampleRate() / resource->mAudioInterface->mSampleRate);
                
                // one gain
                mGains.emplace_back(std::make_unique<Gain>(resource->mAudioInterface->getNodeManager()));
                mGains[0]->setGain(resource->mGain);
                mGains[0]->audioInput.connect(mPlayers[0]->audioOutput);
                
                // the stereo panner to pan the mono source to a stereo signal
                mPanner = std::make_unique<StereoPanner>(resource->mAudioInterface->getNodeManager());
                mPanner->setPanning(resource->mPanning);
                mPanner->leftInput.connect(mGains[0]->audioOutput);
                mPanner->rightInput.connect(mGains[0]->audioOutput);
                
                // two outputs (stereo output)
                for (auto i = 0; i < 2; ++i)
                {
                    mOutputs.emplace_back(std::make_unique<OutputNode>(resource->mAudioInterface->getNodeManager()));
                    mOutputs[i]->setOutputChannel(i);
                }
                mOutputs[0]->audioInput.connect(mPanner->leftOutput);
                mOutputs[1]->audioInput.connect(mPanner->rightOutput);
            }
            
            // Stereo mode
            else if (resource->mAudioFile->getChannelCount() > 1)
            {
                for (auto i = 0; i < 2; ++i)
                {
                    // two players for stereo playback
                    mPlayers.emplace_back(std::make_unique<BufferPlayer>(resource->mAudioInterface->getNodeManager()));
                    
                    mPlayers[i]->play(resource->mAudioFile->getBuffer()[i], 0, resource->mAudioFile->getSampleRate() / resource->mAudioInterface->mSampleRate);
                    
                    // two gains to scale both channels
                    mGains.emplace_back(std::make_unique<Gain>(resource->mAudioInterface->getNodeManager()));
                    mGains[i]->setGain(resource->mGain);
                    mGains[i]->audioInput.connect(mPlayers[i]->audioOutput);
                    
                }
                
                // the stereo panner
                mPanner = std::make_unique<StereoPanner>(resource->mAudioInterface->getNodeManager());
                mPanner->setPanning(resource->mPanning);
                mPanner->leftInput.connect(mGains[0]->audioOutput);
                mPanner->rightInput.connect(mGains[1]->audioOutput);
                
                // two outputs for stereo output
                for (auto i = 0; i < 2; ++i)
                {
                    mOutputs.emplace_back(std::make_unique<OutputNode>(resource->mAudioInterface->getNodeManager()));
                    mOutputs[i]->setOutputChannel(i);
                }
                mOutputs[0]->audioInput.connect(mPanner->leftOutput);
                mOutputs[1]->audioInput.connect(mPanner->rightOutput);
            }
            
            resource->mAudioInterface->getNodeManager().execute([&, resource](){
                mOutputs[0]->setActive(true);
                mOutputs[1]->setActive(true);
            });
            
            return true;
        }
        
        
    }
    
}
