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
    RTTI_PROPERTY("Duration", &nap::audio::PlaybackComponent::mDuration, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("FadeInTime", &nap::audio::PlaybackComponent::mFadeInTime, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("FadeOutTime", &nap::audio::PlaybackComponent::mFadeOutTime, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Pitch", &nap::audio::PlaybackComponent::mPitch, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::PlaybackComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
    RTTI_FUNCTION("start", &nap::audio::PlaybackComponentInstance::start)
    RTTI_FUNCTION("stop", &nap::audio::PlaybackComponentInstance::stop)
    RTTI_FUNCTION("setGain", &nap::audio::PlaybackComponentInstance::setGain)
    RTTI_FUNCTION("setStereoPanning", &nap::audio::PlaybackComponentInstance::setStereoPanning)
    RTTI_FUNCTION("setFadeInTime", &nap::audio::PlaybackComponentInstance::setFadeInTime)
    RTTI_FUNCTION("setFadeOutTime", &nap::audio::PlaybackComponentInstance::setFadeOutTime)
    RTTI_FUNCTION("isStereo", &nap::audio::PlaybackComponentInstance::isStereo)
    RTTI_FUNCTION("isPlaying", &nap::audio::PlaybackComponentInstance::isPlaying)
    RTTI_FUNCTION("getGain", &nap::audio::PlaybackComponentInstance::getGain)
    RTTI_FUNCTION("getStereoPanning", &nap::audio::PlaybackComponentInstance::getStereoPanning)
    RTTI_FUNCTION("getFadeInTime", &nap::audio::PlaybackComponentInstance::getFadeInTime)
    RTTI_FUNCTION("getFadeOutTime", &nap::audio::PlaybackComponentInstance::getFadeOutTime)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
    
        bool PlaybackComponentInstance::init(utility::ErrorState& errorState)
        {
            mResource = getComponent<PlaybackComponent>();
            mGain = mResource->mGain;
            mStereoPanning = mResource->mStereoPanning;
            mFadeInTime = mResource->mFadeInTime;
            mFadeOutTime = mResource->mFadeOutTime;
            mPitch = mResource->mPitch;
            
            mAudioService = getEntityInstance()->getCore()->getService<AudioService>();
            mNodeManager = &mAudioService->getNodeManager();
            
            for (auto channel = 0; channel < mResource->mChannelRouting.size(); ++channel)
            {
                if (mResource->mChannelRouting[channel] >= mResource->mBuffer->getChannelCount())
                {
                    errorState.fail("%s: Routed channel is out of buffer's channel bounds", mResource->mID.c_str());
                    return false;
                }
                
                auto bufferPlayer = mNodeManager->makeSafe<BufferPlayerNode>(*mNodeManager);
                auto gain = mNodeManager->makeSafe<MultiplyNode>(*mNodeManager);
                auto gainControl = mNodeManager->makeSafe<ControlNode>(*mNodeManager);
                
                bufferPlayer->setBuffer(mResource->mBuffer->getBuffer());

                gain->inputs.connect(bufferPlayer->audioOutput);
                gain->inputs.connect(gainControl->output);                
                gainControl->setValue(0);
                SafePtr<BufferPlayerNode> bufferPlayerPtr = bufferPlayer.get();
                gainControl->rampFinishedSignal.connect([&, bufferPlayerPtr](ControlNode& gainControl){
                    if (gainControl.getValue() <= 0)
                        if (!mPlaying)
                            bufferPlayerPtr->stop();
                });
                
                mBufferPlayers.emplace_back(std::move(bufferPlayer));
                mGainNodes.emplace_back(std::move(gain));
                mGainControls.emplace_back(std::move(gainControl));
            }            
            
            if (mResource->mAutoPlay)
                start(mResource->mStartPosition, mResource->mDuration);
            
            return true;
        }
        
        
        void PlaybackComponentInstance::update(double deltaTime)
        {
            if (mPlaying)
            {
                mCurrentPlayingTime += deltaTime * 1000.f;
                if (mCurrentPlayingTime > mDuration - mFadeOutTime)
                    stop();
            }
        }
        
        
        void PlaybackComponentInstance::stop()
        {
            if (!mPlaying)
                return;
            
            mPlaying = false;
            for (auto& gainControl : mGainControls)
                gainControl->ramp(0, mFadeOutTime, RampMode::Linear);
        }
        
        
        void PlaybackComponentInstance::setGain(ControllerValue gain)
        {
            if (gain == mGain)
                return;
            mGain = gain;
            if (mPlaying)
                applyGain(5);
        }
        
        
        void PlaybackComponentInstance::setStereoPanning(ControllerValue panning)
        {
            if (panning == mStereoPanning)
                return;
            mStereoPanning = panning;
            if (mPlaying)
                applyGain(5);
        }
        
        
        void PlaybackComponentInstance::setFadeInTime(TimeValue time)
        {
            mFadeInTime = time;
        }
        
        
        void PlaybackComponentInstance::setFadeOutTime(TimeValue time)
        {
            mFadeOutTime = time;
        }
        
        
        void PlaybackComponentInstance::setPitch(ControllerValue pitch)
        {
            if (pitch == mPitch)
                return;
            
            mPitch = pitch;
            ControllerValue actualSpeed = mPitch * mResource->mBuffer->getSampleRate() / mNodeManager->getSampleRate();
            for (auto& bufferPlayer : mBufferPlayers)
                bufferPlayer->setSpeed(actualSpeed);
        }
        
        
        void PlaybackComponentInstance::applyGain(TimeValue rampTime)
        {
            if (mResource->isStereo())
            {
                ControllerValue left = 0;
                ControllerValue right = 0;
                equalPowerPan(mStereoPanning, left, right);
                mGainControls[0]->ramp(left * mGain, rampTime);
                mGainControls[1]->ramp(right * mGain, rampTime);
            }
            else {
                for (auto& gainControl : mGainControls)
                    gainControl->ramp(mGain, rampTime);
            }
        }


        void PlaybackComponentInstance::start(TimeValue startPosition, TimeValue duration)
        {
            ControllerValue actualSpeed = mPitch * mResource->mBuffer->getSampleRate() / mNodeManager->getSampleRate();
            if (duration == 0)
                mDuration = mResource->mBuffer->getSize();
            else
                mDuration = duration;
            mCurrentPlayingTime = 0;
            for (auto channel = 0; channel < mBufferPlayers.size(); ++channel)
            {
                if (mBufferPlayers[channel] != nullptr)
                    mBufferPlayers[channel]->play(mResource->mChannelRouting[channel], startPosition * mNodeManager->getSamplesPerMillisecond(), actualSpeed);
            }
            applyGain(mFadeInTime);
            mPlaying = true;
        }
        
    }
    
}
