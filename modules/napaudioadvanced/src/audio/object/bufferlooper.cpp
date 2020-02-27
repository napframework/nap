#include "bufferlooper.h"

RTTI_BEGIN_STRUCT(nap::audio::BufferLooper::Settings)
    RTTI_PROPERTY("Buffer", &nap::audio::BufferLooper::Settings::mBufferResource, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Loop", &nap::audio::BufferLooper::Settings::mLoop, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Start", &nap::audio::BufferLooper::Settings::mStart, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("LoopStart", &nap::audio::BufferLooper::Settings::mLoopStart, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("LoopEnd", &nap::audio::BufferLooper::Settings::mLoopEnd, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Transpose", &nap::audio::BufferLooper::Settings::mTranspose, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("CrossFadeTime", &nap::audio::BufferLooper::Settings::mCrossFadeTime, nap::rtti::EPropertyMetaData::Required)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(nap::audio::BufferLooper)
    RTTI_PROPERTY("Settings", &nap::audio::BufferLooper::mSettings, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("ChannelCount", &nap::audio::BufferLooper::mChannelCount, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("AutoPlay", &nap::audio::BufferLooper::mAutoPlay, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("EqualPowerTable", &nap::audio::BufferLooper::mEqualPowerTable, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        std::unique_ptr<AudioObjectInstance> BufferLooper::createInstance(NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            auto instance = std::make_unique<BufferLooperInstance>();
            if (!instance->init(mSettings, mEqualPowerTable, mChannelCount, mAutoPlay, nodeManager, errorState))
                return nullptr;
            
            return std::move(instance);
        }
        
        
        bool BufferLooper::Settings::init(utility::ErrorState& errorState)
        {
            if (mBufferResource == nullptr)
            {
                errorState.fail("Invalid BufferLooper settings: no buffer given");
                return false;
            }
            
            if (mStart < 0.f || mStart >= mBufferResource->toMilliseconds(mBufferResource->getSize()))
            {
                errorState.fail("Invalid BufferLooper settings: invalid start position");
                return false;
            }
            
            if (mLoopStart < 0.f || mLoopStart >= mBufferResource->toMilliseconds(mBufferResource->getSize()))
            {
                errorState.fail("Invalid BufferLooper settings: invalid loop start position");
                return false;
            }
            
            if (mLoopEnd < 0.f || mLoopEnd >= mBufferResource->toMilliseconds(mBufferResource->getSize()) || mLoopEnd <= mLoopStart)
            {
                errorState.fail("Invalid BufferLooper settings: invalid loop end position");
                return false;
            }
            
            mFirstSustainDuration = mLoopEnd - mStart - mCrossFadeTime;
            if (mFirstSustainDuration < 0)
            {
                errorState.fail("Invalid BufferLooper settings: start before end or crossfade time too long.");
                return false;
            }
            
            mLoopSustainDuration = mLoopEnd - mLoopStart - mCrossFadeTime * 2;
            if (mLoopSustainDuration < 0)
            {
                errorState.fail("Invalid BufferLooper settings: loop start before end or crossfade time too long.");
                return false;
            }
            
            return true;
        }
        

        bool BufferLooper::init(utility::ErrorState& errorState)
        {
            
            return true;
        }


        bool BufferLooperInstance::init(BufferLooper::Settings& settings, ResourcePtr<EqualPowerTable> equalPowerTable, int channelCount, bool autoPlay, NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            mSettings = settings;
            
            if (!mSettings.init(errorState))
                return false;
            
            mBufferPlayer = std::make_unique<BufferPlayer>();
            mBufferPlayer->mID = "BufferPlayer";
            mBufferPlayer->mAutoPlay = false;
            mBufferPlayer->mBufferResource = mSettings.mBufferResource;
            mBufferPlayer->mChannelCount = channelCount;
            if (!mBufferPlayer->init(errorState))
            {
                errorState.fail("Failed to initialize BufferLooper " + getName());
                return false;
            }
            
            EnvelopeNode::Segment attack;
            attack.mDestination = 1.f;
            attack.mDuration = 0.f;
            attack.mDurationRelative = false;
            attack.mTranslate = true;
            attack.mMode = RampMode::Linear;
            
            EnvelopeNode::Segment sustain;
            sustain.mDestination = 1.f;
            sustain.mDuration = mSettings.getFirstSustainDuration();
            sustain.mDurationRelative = false;
            sustain.mTranslate = false;
            sustain.mMode = RampMode::Linear;
            
            EnvelopeNode::Segment decay;
            decay.mDestination = 0.f;
            decay.mDuration = mSettings.mCrossFadeTime;
            decay.mDurationRelative = false;
            decay.mTranslate = true;
            decay.mMode = RampMode::Linear;
            
            mEnvelope = std::make_unique<Envelope>();
            mEnvelope->mID = "Envelope";
            mEnvelope->mSegments.emplace_back(attack);
            mEnvelope->mSegments.emplace_back(sustain);
            mEnvelope->mSegments.emplace_back(decay);
            mEnvelope->mAutoTrigger = false;
            mEnvelope->mEqualPowerTable = equalPowerTable;
            if (!mEnvelope->init(errorState))
            {
                errorState.fail("Failed to initialize BufferLooper " + getName());
                return false;
            }
            
            mGain = std::make_unique<Gain>();
            mGain->mID = "Gain";
            mGain->mChannelCount = channelCount;
            mGain->mInputs.emplace_back(mBufferPlayer.get());
            mGain->mInputs.emplace_back(mEnvelope.get());
            if (!mGain->init(errorState))
            {
                errorState.fail("Failed to initialize BufferLooper " + getName());
                return false;
            }
            
            mVoice = std::make_unique<Voice>();
            mVoice->mID = "Voice";
            mVoice->mObjects.emplace_back(mBufferPlayer.get());
            mVoice->mObjects.emplace_back(mGain.get());
            mVoice->mObjects.emplace_back(mEnvelope.get());
            mVoice->mEnvelope = mEnvelope.get();
            mVoice->mOutput = mGain.get();
            if (!mVoice->init(errorState))
            {
                errorState.fail("Failed to initialize BufferLooper " + getName());
                return false;
            }
            
            mPolyphonic = std::make_unique<PolyphonicObject>();
            mPolyphonic->mID = "Polyphonic";
            mPolyphonic->mVoice = mVoice.get();
            mPolyphonic->mChannelCount = channelCount;
            mPolyphonic->mVoiceCount = 2;
            mPolyphonic->mVoiceStealing = false;
            if (!mPolyphonic->init(errorState))
            {
                errorState.fail("Failed to initialize BufferLooper " + getName());
                return false;
            }

            mPolyphonicInstance = mPolyphonic->instantiate<PolyphonicObjectInstance>(nodeManager, errorState);
            
            if (autoPlay)
                start();
            
            return true;
        }
        
        
        void BufferLooperInstance::start()
        {
            startVoice(true);
        }
        
        
        void BufferLooperInstance::start(BufferLooper::Settings& settings)
        {
            mSettings = settings;
            startVoice(true);
        }

        
        void BufferLooperInstance::stop()
        {
            for (auto voice : mVoices)
                voice->stop();
        }
        
        
        void BufferLooperInstance::segmentFinished(EnvelopeNode& envelope)
        {
            if (envelope.getCurrentSegment() == 1)
            {
                startVoice(false);
            }
        }
        
        
        void BufferLooperInstance::startVoice(bool fromStart)
        {
            auto voice = mPolyphonicInstance->findFreeVoice();
            assert(voice != nullptr);
            mVoices.emplace(voice);
            auto bufferPlayer = voice->getObject<MultiChannelInstance<BufferPlayerNode>>("BufferPlayer");
            auto& envelope = voice->getEnvelope();
            
            for (auto channel = 0; channel < bufferPlayer->getChannelCount(); ++channel)
            {
                auto bufferPlayerChannel = bufferPlayer->getChannel(channel);
                bufferPlayerChannel->stop();
                bufferPlayerChannel->setBuffer(mSettings.mBufferResource->getBuffer());
            }
            
            envelope.getSegmentFinishedSignal().disconnect(segmentFinishedSlot); // We need to disconnect first to avoid connecting to the same signal twice.
            envelope.getSegmentFinishedSignal().connect(segmentFinishedSlot);
            
            auto speed = mtof(64.f + mSettings.mTranspose) / mtof(64.f);
            
            if (fromStart)
            {
                envelope.setSegmentData(0, 0, 1.f, false, false, false);
                envelope.setSegmentData(1, mSettings.getFirstSustainDuration() * speed, 1.f, false, false, true);
                envelope.setSegmentData(2, mSettings.mCrossFadeTime * speed, 0.f, false, false, true);
                for (auto channel = 0; channel < getChannelCount(); ++channel)
                {
                    bufferPlayer->getChannel(channel)->play(channel, mSettings.mBufferResource->toSamples(mSettings.mStart), speed);
                }
            }
            else {
                envelope.setSegmentData(0, mSettings.mCrossFadeTime * speed, 1.f, false, false, true);
                envelope.setSegmentData(1, mSettings.getLoopSustainDuration() * speed, 1.f, false, false, true);
                envelope.setSegmentData(2, mSettings.mCrossFadeTime * speed, 0.f, false, false, true);
                for (auto channel = 0; channel < getChannelCount(); ++channel)
                {
                    bufferPlayer->getChannel(channel)->play(channel, mSettings.mBufferResource->toSamples(mSettings.mStart), speed);
                }
            }
            
            mPolyphonicInstance->play(voice);
        }
        
        
    }

    
}
