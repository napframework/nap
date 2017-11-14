#pragma once

// Audio includes
#include <node/controlnode.h>

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
                TimeValue duration = 0;
                ControllerValue destination = 0;
                bool durationRelative = false; //** this indicates wether the duration of this segment is relative to the total duration of the envelope.
                ControlNode::RampMode mode = ControlNode::RampMode::LINEAR; //** This indicates the line shape of the segment
            };
            using Envelope = std::vector<Segment>;

        public:
            EnvelopeGenerator(NodeManager& manager);
            
            /**
             * Triggers an envelope.
             * @param totalDuration: if this value is greater than the total of all durations of segments that have durationRelative = false
                 the resting time wille be divided over the segments with durationRelative = true, using their duration values as denominator.
             */
            void trigger(Envelope& envelope, TimeValue totalDuration = 0);
            
            /**
             * Triggers a section of an envelope.
             * @param totalDuration: if this value is greater than the total of all durations of segments that have durationRelative = false
             * @param startSegment: the start segment of the envelope section to be played
             * @param endSegment: the end segment of the envelope section to be played
             * @param startValue: the startValue of the line when the section is triggered.
             * @param totalDuration: if this value is greater than the total of all durations of segments that have durationRelative = false
             the resting time wille be divided over the segments with durationRelative = true, using their duration values as denominator.
             */
            void trigger(Envelope& envelope, int startSegment, int endSegment, ControllerValue startValue = 0, TimeValue totalDuration = 0);
            
            /**
             * Stops playback of the envelope generator by fading the signal out to zero in @rampTime milliseconds.
             */
            void stop(TimeValue rampTime = 5);
            
            /**
             * This signal is emitted whenever an envelope finishes playing and the output signal has reached zero.
             */
            nap::Signal<EnvelopeGenerator&> envelopeFinishedSignal;
            
        private:
            void playSegment(int index);
            
            nap::Slot<ControlNode&> rampFinishedSlot = { this, &EnvelopeGenerator::rampFinished };
            void rampFinished(ControlNode&);
            
            int mCurrentSegment = 0;
            int mEndSegment = 0;
            Envelope* mEnvelope = nullptr;
            TimeValue mTotalRelativeDuration = 0;
        };
        
    }
    
}
