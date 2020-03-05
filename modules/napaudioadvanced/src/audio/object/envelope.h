#pragma once

// Nap includes
#include <rtti/objectptr.h>
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/audioobject.h>
#include <audio/node/envelopenode.h>
#include <audio/resource/equalpowertable.h>

namespace nap
{

    namespace audio
    {

        class EnvelopeInstance;

        /**
         * Audio object resource for an envelope generator.
         */
        class NAPAPI Envelope : public AudioObject
        {
            RTTI_ENABLE(AudioObject)
        public:
            Envelope() = default;

            EnvelopeNode::Envelope mSegments; ///< The segments that define the envelope's shape.
            bool mAutoTrigger = false; ///< If true the envelope will be triggered automatically on initialization.
            ResourcePtr<EqualPowerTable> mEqualPowerTable = nullptr; ///< Property: 'EqualPowerTable' used to translate to equal power curve.

        private:
            // Inherited from AudioObject
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
        };


        /**
         * Instance of an envelope generator.
         */
        class NAPAPI EnvelopeInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
        public:
            EnvelopeInstance() : AudioObjectInstance() { }
            EnvelopeInstance(const std::string& name) : AudioObjectInstance(name) { }

            // Inherited from AudioObjectInstance
            bool init(EnvelopeNode::Envelope segments, bool autoTrigger, NodeManager& nodeManager, audio::SafePtr<Translator<float>> translator, utility::ErrorState& errorState);
            
            OutputPin* getOutputForChannel(int channel) override { return &mEnvelopeGenerator->output; }
            int getChannelCount() const override { return 1; }

            /**
             * Triggers the envelope to start playing from the start segment.
             * If @totalDuration does not equal zero the relative durations in the segments will be scaled in order to get the total duration of the envelope to match this parameter.
             */
            void trigger(TimeValue totalDuration = 0)
            {
                mEnvelopeGenerator->trigger(totalDuration);
            }
            
            
            /**
             * Triggers a section of an envelope.
             * @param totalDuration: if this value is greater than the total of all durations of segments that have durationRelative = false
             * @param startSegment: the start segment of the envelope section to be played
             * @param endSegment: the end segment of the envelope section to be played
             * @param startValue: the startValue of the line when the section is triggered.
             * @param totalDuration: if this value is greater than the total of all durations of segments that have durationRelative = false
             the resting time wille be divided over the segments with durationRelative = true, using their duration values as denominator.
             */
            void triggerSection(int startSegment, int endSegment, ControllerValue startValue = 0, TimeValue totalDuration = 0)
            {
                mEnvelopeGenerator->trigger(startSegment, endSegment, startValue, totalDuration);
            }

            /**
             * Stops playing the envelope by fading to zero within @rampTime.
             */
            void stop(TimeValue rampTime) { mEnvelopeGenerator->stop(rampTime); }

            /**
             * Sets the envelope data for one segment of the envelope. The segment index specifies which segment will be edited.
             * If the index out of bounds no action will be taken.
             */
            void setSegmentData(unsigned int segmentIndex, TimeValue duration, ControllerValue destination, bool durationRelative, bool exponential, bool useTranslator);
            
            /**
             * Assigns new envelope data
             */
            void setEnvelopeData(const EnvelopeNode::Envelope& envelope) { mEnvelopeGenerator->getEnvelope() = envelope; }

            /**
             * Returns the current output value of the envelope generator.
             */
            ControllerValue getValue() const { return mEnvelopeGenerator->getValue(); }

            /**
             * Returns a signal that will be emitted when the total envelope shape has finished and the generator outputs zero again.
             */
            nap::Signal<EnvelopeNode&>& getEnvelopeFinishedSignal() { return mEnvelopeGenerator->envelopeFinishedSignal; }

            /**
             * Returns a sginal that will be emitted when one segment of the envelope has finished playing. The semgent index of the ENvelopeGenerator still contains the number of the segment that has just finished.
             */
            nap::Signal<EnvelopeNode&>& getSegmentFinishedSignal() { return mEnvelopeGenerator->segmentFinishedSignal; }
            
        private:
            SafeOwner<EnvelopeNode> mEnvelopeGenerator = nullptr;
            SafePtr<Translator<ControllerValue>> mTranslator = nullptr;
        };


    }

}
