#pragma once

// Std includes
#include <atomic>

// Audio includes
#include <audio/utility/audiotypes.h>
#include <audio/core/audionode.h>
#include <audio/utility/safeptr.h>
#include <audio/utility/dirtyflag.h>
#include <audio/utility/rampedvalue.h>
#include <audio/utility/translator.h>

// Nap includes
#include <nap/signalslot.h>

namespace nap
{

    namespace audio
    {

        /**
         * Envelope generator that can trigger envelopes to generate a control signal.
         * Envelopes are specified as an array of segments with a duration and a destination value.
         */
        class NAPAPI EnvelopeNode : public Node {
            RTTI_ENABLE(Node)
        public:
            /**
             * An envelope is a vector of segments.
             * A segment describes a line segment consisting of a duration and a destination value.
             */
            struct NAPAPI Segment
            {
                TimeValue mDuration = 0;
                ControllerValue mDestination = 0;
                bool mDurationRelative = false; ///< this indicates wether the duration of this segment is relative to the total duration of the envelope.
                RampMode mMode = RampMode::Linear; ///< This indicates the line shape of the segment
                bool mTranslate = false; ///< Indicates wether the output value will be translated using a lookup table.
            };
            using Envelope = std::vector<Segment>;

        public:
            /**
             * Note that this constructor differs from the standard @Node ctor signature which just takes a @NodeManager.
             * This is fine because EnvelopeNode will not need to be wrapped a @NodeObject but in an @Envelope.
             * @param manager
             * @param envelope vector of @Segment with segments of the envelopes that will be generated.
             * @param translator SafePtr to a @Translator object that manages a function to translate the envelope's output for certain segments. Generally contains an @EqualPowerTranslator.
             */
            EnvelopeNode(NodeManager& manager, const Envelope& envelope, SafePtr<Translator<ControllerValue>> translator);

            /**
             * The output signal pin
             */
            OutputPin output = { this };

            /**
             * Triggers an envelope.
             * @param totalDuration: if this value is greater than the total of all durations of segments that have durationRelative = false
                 the resting time wille be divided over the segments with durationRelative = true, using their duration values as denominator.
             */
            void trigger(TimeValue totalDuration = 0);

            /**
             * Triggers a section of an envelope.
             * @param totalDuration: if this value is greater than the total of all durations of segments that have durationRelative = false
             * @param startSegment: the start segment of the envelope section to be played
             * @param endSegment: the end segment of the envelope section to be played
             * @param startValue: the startValue of the line when the section is triggered.
             * @param totalDuration: if this value is greater than the total of all durations of segments that have durationRelative = false
             the resting time wille be divided over the segments with durationRelative = true, using their duration values as denominator.
             */
            void trigger(int startSegment, int endSegment, ControllerValue startValue = 0, TimeValue totalDuration = 0);

            /**
             * Stops playback of the envelope generator by fading the signal out to zero in @rampTime milliseconds.
             */
            void stop(TimeValue rampTime = 5);

            /**
             * Returns the current output value of the envelope generator.
             */
            ControllerValue getValue() const { return mCurrentValue.load(); }

            /**
             * This signal is emitted whenever an envelope finishes playing and the output signal has reached zero.
             */
            nap::Signal<EnvelopeNode&> envelopeFinishedSignal;
            
            /**
             * This signal is emitted whenever a segment of the envelope has finfished playing.
             */
            nap::Signal<EnvelopeNode&> segmentFinishedSignal;
            
            /**
             * Use this to edit the envelope data. Handle with care and don't use this while the EnvelopeGenerator is playing!
             */
            Envelope& getEnvelope() { return mEnvelope; }
            
            /**
             * Request the current the index of the segment that is currently playing in the envelope.
             */
            int getCurrentSegment() { return mCurrentSegment; }
            
        private:
            void process() override;

            void playSegment(int index);
            void updateEnvelope();

            nap::Slot<ControllerValue> rampFinishedSlot = { this, &EnvelopeNode::rampFinished };
            void rampFinished(ControllerValue);

            int mCurrentSegment = { 0 };
            int mEndSegment = { 0 };
            Envelope mEnvelope; // 1000ms attack and 1000ms decay

            RampedValue<ControllerValue> mValue = { 0.f };
            std::atomic<ControllerValue> mCurrentValue = { 0.f };
            bool mTranslate = false;

            std::atomic<int> mNewCurrentSegment = { 0 };
            std::atomic<int> mNewEndSegment = { 0 };
            std::atomic<TimeValue> mFadeOutTime = { 0.f };
            SafePtr<Translator<ControllerValue>> mTranslator = nullptr; // Helper object to apply a translation to the output value.
            DirtyFlag mIsDirty;

            TimeValue mTotalRelativeDuration = 0;
        };

    }

}
