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
            resource = getComponent<PlaybackComponent>();
            mGain = resource->mGain;
            mStereoPanning = resource->mStereoPanning;
            mFadeInTime = resource->mFadeInTime;
            mFadeOutTime = resource->mFadeOutTime;
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
                        if (!mPlaying)
                            bufferPlayerPtr->stop();
                });
                
                mBufferPlayers.emplace_back(std::move(bufferPlayer));
                mGainNodes.emplace_back(std::move(gain));
                mGainControls.emplace_back(std::move(gainControl));
            }
            
            
            if (resource->mAutoPlay)
                start(resource->mStartPosition, resource->mDuration);
            
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
        
        
        void PlaybackComponentInstance::start(TimeValue startPosition, TimeValue duration)
        {
            ControllerValue actualSpeed = mPitch * resource->mBuffer->getSampleRate() / nodeManager->getSampleRate();
            nodeManager->execute([&, actualSpeed, startPosition, duration](){
                mPlaying = true;
                if (duration == 0)
                    mDuration = resource->mBuffer->getSize();
                else
                    mDuration = duration;
                mCurrentPlayingTime = 0;
                for (auto channel = 0; channel < mBufferPlayers.size(); ++channel)
                {
                    mBufferPlayers[channel]->play(resource->mBuffer->getBuffer()[resource->mChannelRouting[channel]], startPosition * nodeManager->getSamplesPerMillisecond(), actualSpeed);
                }
                applyGain(mFadeInTime);
            });
        }
        
        
        void PlaybackComponentInstance::stop()
        {
            if (!mPlaying)
                return;
            
            nodeManager->execute([&](){
                mPlaying = false;
                for (auto& gainControl : mGainControls)
                    gainControl->ramp(0, mFadeOutTime, ControlNode::RampMode::EXPONENTIAL);
            });
        }
        
        
        void PlaybackComponentInstance::setGain(ControllerValue gain)
        {
            nodeManager->execute([&](){
                mGain = gain;
                if (mPlaying)
                    applyGain(5);
            });
        }
        
        
        void PlaybackComponentInstance::setStereoPanning(ControllerValue panning)
        {
            nodeManager->execute([&](){
                mStereoPanning = panning;
                if (mPlaying)
                    applyGain(5);
            });
        }
        
        
        void PlaybackComponentInstance::setFadeInTime(TimeValue time)
        {
            mFadeInTime = time;
        }
        
        
        void PlaybackComponentInstance::setFadeOutTime(TimeValue time)
        {
            mFadeOutTime = time;
        }
        
        
        void PlaybackComponentInstance::applyGain(TimeValue rampTime)
        {
            if (resource->isStereo())
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

        
    }
    
}
