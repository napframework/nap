#pragma once

// Std includes
#include <atomic>

// Audio includes
#include <audio/node/controlnode.h>
#include <audio/utility/safeptr.h>
#include <audio/utility/dirtyflag.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Envelope generator that can trigger envelopes to generate a control signal.
         * Envelopes are specified as an array of segments with a duration and a destination value.
         */
        class NAPAPI EnvelopeGenerator : public ControlNode {
            
        public:
            /**
             * An envelope is a vector of segments.
             * A segment describes a line segment consisting of a duration and a destination value.
             */
            struct NAPAPI Segment
            {
                TimeValue mDuration = 0;
                ControllerValue mDestination = 0;
                bool mDurationRelative = false; //** this indicates wether the duration of this segment is relative to the total duration of the envelope.
                RampMode mMode = RampMode::Linear; //** This indicates the line shape of the segment
            };
            using Envelope = std::vector<Segment>;

        public:
            EnvelopeGenerator(NodeManager& manager);
            
            /**
             * Triggers an envelope.
			 * @param envelope envelope to trigger.
             * @param totalDuration: if this value is greater than the total of all durations of segments that have durationRelative = false
             * the resting time will be divided over the segments with durationRelative = true, using their duration values as denominator.
             */
            void trigger(SafePtr<Envelope> envelope, TimeValue totalDuration = 0);
            
            /**
             * Triggers a section of an envelope.
             * @param envelope: envelope to trigger
             * @param startSegment: the start segment of the envelope section to be played
             * @param endSegment: the end segment of the envelope section to be played
             * @param startValue: the startValue of the line when the section is triggered.
             * @param totalDuration: if this value is greater than the total of all durations of segments that have durationRelative = false
             the resting time wille be divided over the segments with durationRelative = true, using their duration values as denominator.
             */
            void trigger(SafePtr<Envelope> envelope, int startSegment, int endSegment, ControllerValue startValue = 0, TimeValue totalDuration = 0);
            
            /**
             * Stops playback of the envelope generator by fading the signal out to zero in milliseconds based on the given rampTime.
			 * @param rampTime time in milliseconds
             */
            void stop(TimeValue rampTime = 5);
            
            /**
             * This signal is emitted whenever an envelope finishes playing and the output signal has reached zero.
             */
            nap::Signal<EnvelopeGenerator&> envelopeFinishedSignal;
            
        private:
            void playSegment(int index);
            void update();
            
            nap::Slot<ControlNode&> rampFinishedSlot = { this, &EnvelopeGenerator::rampFinished };
            void rampFinished(ControlNode&);
            
            int mCurrentSegment = { 0 };
            int mEndSegment = { 0 };
            Envelope* mEnvelope = nullptr;
            
            std::atomic<int> mNewCurrentSegment = { 0 };
            std::atomic<int> mNewEndSegment = { 0 };
            SafePtr<Envelope> mNewEnvelope = nullptr;
            DirtyFlag mIsDirty;

            TimeValue mTotalRelativeDuration = 0;
        };
        
    }
    
}
