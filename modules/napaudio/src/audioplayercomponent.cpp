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
        
        bool AudioPlayerComponentInstance::init(const ObjectPtr<Component>& resourcePtr, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
        {
            AudioPlayerComponent* resource = rtti_cast<AudioPlayerComponent>(resourcePtr.get());
            
            // Mono mode
            if (resource->mAudioFile->getBuffer().getChannelCount() == 1)
            {
                mPlayers.emplace_back(std::make_unique<BufferPlayer>(resource->mAudioInterface->getNodeManager()));
                for (auto i = 0; i < 2; ++i)
                {
                    mGains.emplace_back(std::make_unique<Gain>(resource->mAudioInterface->getNodeManager()));
                    mOutputs.emplace_back(std::make_unique<AudioOutputNode>(resource->mAudioInterface->getNodeManager()));
                    mGains[i]->audioInput.connect(mPlayers[0]->audioOutput);
                    mGains[i]->setGain(resource->mGain);
                    mOutputs[i]->setOutputChannel(i);
                }
                mPanner = std::make_unique<StereoPanner>(resource->mAudioInterface->getNodeManager());
                mPanner->leftInput.connect(mPlayers[0]->audioOutput);
                mPanner->rightInput.connect(mPlayers[0]->audioOutput);
                mOutputs[0]->audioInput.connect(mPanner->leftOutput);
                mOutputs[1]->audioInput.connect(mPanner->rightOutput);
                mPanner->setPanning(resource->mPanning);
                mPlayers[0]->play(resource->mAudioFile->getBuffer()[0], 0, resource->mAudioFile->getSampleRate() / resource->mAudioInterface->mSampleRate);
                
            }
            
            // Stereo mode
            if (resource->mAudioFile->getChannelCount() > 1)
            {
                for (auto i = 0; i < 2; ++i)
                {
                    mPlayers.emplace_back(std::make_unique<BufferPlayer>(resource->mAudioInterface->getNodeManager()));
                    mGains.emplace_back(std::make_unique<Gain>(resource->mAudioInterface->getNodeManager()));
                    mOutputs.emplace_back(std::make_unique<AudioOutputNode>(resource->mAudioInterface->getNodeManager()));
                    mGains[i]->audioInput.connect(mPlayers[i]->audioOutput);
                    mGains[i]->setGain(resource->mGain);
                    mOutputs[i]->setOutputChannel(i);
                    mPlayers[i]->play(resource->mAudioFile->getBuffer()[i], 0, resource->mAudioFile->getSampleRate() / resource->mAudioInterface->mSampleRate);
                }
                mPanner = std::make_unique<StereoPanner>(resource->mAudioInterface->getNodeManager());
                mPanner->leftInput.connect(mPlayers[0]->audioOutput);
                mPanner->rightInput.connect(mPlayers[1]->audioOutput);
                mOutputs[0]->audioInput.connect(mPanner->leftOutput);
                mOutputs[1]->audioInput.connect(mPanner->rightOutput);
                mPanner->setPanning(resource->mPanning);
            }
            
            return true;
        }
        
        
    }
    
}
