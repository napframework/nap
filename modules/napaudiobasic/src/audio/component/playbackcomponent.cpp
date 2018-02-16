#include "playbackcomponent.h"

// Nap includes
#include <entity.h>
#include <nap/core.h>

// Audio includes
#include <audio/service/audioservice.h>


// RTTI
RTTI_BEGIN_CLASS(nap::audio::PlaybackComponent)
    RTTI_PROPERTY("Buffer", &nap::audio::PlaybackComponent::mBuffer, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("ChannelRouting", &nap::audio::PlaybackComponent::mChannelRouting, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Gain", &nap::audio::PlaybackComponent::mGain, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("StereoPanning", &nap::audio::PlaybackComponent::mStereoPanning, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("AutoPlay", &nap::audio::PlaybackComponent::mAutoPlay, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("StartPosition", &nap::audio::PlaybackComponent::mStartPosition, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("FadeIn", &nap::audio::PlaybackComponent::mFadeIn, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Pitch", &nap::audio::PlaybackComponent::mPitch, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::PlaybackComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
    RTTI_FUNCTION("isStereo", &nap::audio::PlaybackComponentInstance::isStereo)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
    
        bool PlaybackComponentInstance::init(utility::ErrorState& errorState)
        {
            resource = getComponent<PlaybackComponent>();
            mGain = resource->mGain;
            mPanning = resource->mStereoPanning;
            mStartPosition = resource->mStartPosition;
            mFadeIn = resource->mFadeIn;
            mFadeOut = resource->mFadeOut;
            mPitch = resource->mPitch;
            
            nodeManager = &getEntityInstance()->getCore()->getService<AudioService>(rtti::ETypeCheck::EXACT_MATCH)->getNodeManager();
            
            for (auto channel = 0; channel < resource->mChannelRouting.size(); ++channel)
            {
                if (resource->mChannelRouting[channel] >= resource->mBuffer->getChannelCount())
                {
                    errorState.fail("Routed channel is out of buffer's channel bounds");
                    return false;
                }
                
                auto bufferPlayer = std::make_unique<BufferPlayerNode>(*nodeManager);
                auto gain = std::make_unique<GainNode>(*nodeManager);
                auto gainControl = std::make_unique<ControlNode>(*nodeManager);
                
                gain->inputs.connect(bufferPlayer->audioOutput);
                gain->inputs.connect(gainControl->output);                
                gainControl->setValue(0);
                BufferPlayerNode* bufferPlayerPtr = bufferPlayer.get();
                gainControl->rampFinishedSignal.connect([&, bufferPlayerPtr](ControlNode& gainControl){
                    if (gainControl.getValue() <= 0)
                        if (mStopping)
                            bufferPlayerPtr->stop();
                });
                
                mBufferPlayers.emplace_back(std::move(bufferPlayer));
                mGainNodes.emplace_back(std::move(gain));
                mGainControls.emplace_back(std::move(gainControl));
            }
            
            
            if (resource->mAutoPlay)
                start();
            
            return true;
        }
        
        
        void PlaybackComponentInstance::start()
        {
            mStopping = false;
            ControllerValue actualSpeed = mPitch * resource->mBuffer->getSampleRate() / nodeManager->getSampleRate();
            for (auto channel = 0; channel < mBufferPlayers.size(); ++channel)
            {
                mBufferPlayers[channel]->play(resource->mBuffer->getBuffer()[resource->mChannelRouting[channel]], resource->mStartPosition * nodeManager->getSamplesPerMillisecond(), actualSpeed);
            }
            applyGain();
        }
        
        
        void PlaybackComponentInstance::stop()
        {
            mStopping = true;
            for (auto& gainControl : mGainControls)
                gainControl->ramp(0, mFadeOut);
        }

        
        void PlaybackComponentInstance::applyGain()
        {
            if (resource->isStereo())
            {
                ControllerValue left = 0;
                ControllerValue right = 0;
                equalPowerPan(mPanning, left, right);
                mGainControls[0]->ramp(left * mGain, mFadeIn);
                mGainControls[1]->ramp(right * mGain, mFadeIn);
            }
            else {
                for (auto& gainControl : mGainControls)
                    gainControl->ramp(mGain, mFadeIn);
            }
        }

        
    }
    
}
