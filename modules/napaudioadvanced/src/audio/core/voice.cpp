#include "voice.h"

// Audio includes
#include <audio/core/polyphonicobject.h>

RTTI_BEGIN_CLASS(nap::audio::Voice)
    RTTI_PROPERTY("Envelope", &nap::audio::Voice::mEnvelope, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS


namespace nap
{
    namespace audio
    {
        using VoiceInstanceSignal = nap::Signal<nap::audio::VoiceInstance&>;
    }
}

RTTI_BEGIN_CLASS(nap::audio::VoiceInstanceSignal)
    RTTI_FUNCTION("connect", (void(nap::audio::VoiceInstanceSignal::*)(const pybind11::function))&nap::audio::VoiceInstanceSignal::connect)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::audio::VoiceInstance)
    RTTI_FUNCTION("play", &nap::audio::VoiceInstance::play)
    RTTI_FUNCTION("stop", &nap::audio::VoiceInstance::stop)
    RTTI_FUNCTION("getFinishedSignal", &nap::audio::VoiceInstance::getFinishedSignal)
RTTI_END_CLASS


namespace nap
{
    
    namespace audio
    {
        
        bool VoiceInstance::init(Voice& resource, NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            if (!GraphInstance::init(resource, nodeManager, errorState))
                return false;

            mEnvelope = getObject<EnvelopeInstance>(resource.mEnvelope->mID.c_str());
            if (mEnvelope == nullptr)
            {
                errorState.fail("%s envelope not found", resource.mID.c_str());
                return false;
            }
            
            mEnvelope->getEnvelopeFinishedSignal().connect(envelopeFinishedSlot);
                        
            return true;
        }
        
        
        void VoiceInstance::play(TimeValue duration)
        {
            mEnvelope->trigger(duration);
            mStartTime = getNodeManager().getSampleTime();
        }


        /**
         * Triggers a section of the envelope of the voice.
         * @param totalDuration: if this value is greater than the total of all durations of segments that have durationRelative = false
         * @param startSegment: the start segment of the envelope section to be played
         * @param endSegment: the end segment of the envelope section to be played
         * @param startValue: the startValue of the line when the section is triggered.
         * @param totalDuration: if this value is greater than the total of all durations of segments that have durationRelative = false
         the resting time wille be divided over the segments with durationRelative = true, using their duration values as denominator.
         */
        void VoiceInstance::playSection(int startSegment, int endSegment, ControllerValue startValue, TimeValue totalDuration)
        {
            mEnvelope->triggerSection(startSegment, endSegment, startValue, totalDuration);
            mStartTime = getNodeManager().getSampleTime();
        }

        
        
        void VoiceInstance::stop(TimeValue rampTime)
        {
            mEnvelope->stop(rampTime);
        }
        
        
        bool VoiceInstance::try_use()
        {
            bool expected = false;
            return (mBusy.compare_exchange_strong(expected, true));
        }
        
        
        void VoiceInstance::free()
        {
            bool expected = true;
            while (!mBusy.compare_exchange_weak(expected, false)) { }
        }

        
        void VoiceInstance::envelopeFinished(EnvelopeNode&)
        {
            finishedSignal(*this);
            mBusy = false;
        }


        
    }
    
}
