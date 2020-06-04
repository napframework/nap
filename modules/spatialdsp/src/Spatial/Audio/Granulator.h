#pragma once

#include <audio/object/envelope.h>
#include <audio/object/gain.h>
#include <audio/object/circularbufferplayer.h>
#include <audio/node/outputnode.h>
#include <audio/core/audioobject.h>
#include <audio/core/nodeobject.h>
#include <audio/utility/translator.h>
#include <audio/utility/audiotypes.h>
#include <utility/dllexport.h>

namespace nap
{

    namespace audio
    {

        class Voice;
        class CircularBufferInstance;
        class NestedNodeManagerInstance;
        class PolyphonicInstance;

        /**
        * Audio object instance that implements a multichannel granulation effect.
        * The grains will either be sampled from a @CircularBufferNode or a MultiSampleBuffer that is to be passed to the initialization method.
        */
        class NAPAPI GranulatorInstance : public AudioObjectInstance
        {
        public:
            /**
            * Possible amplitude envelope shapes for a grain
            */
            enum Shape { hanning, expodec, rexpodec };

            /**
            * Manages the DSP for one single channel of the granulator.
            * All the DSP runs on a @NestedNodeManagerNode that can run on a lower internal buffersize for increased timing accuracy.
            */
            class Channel;

        public:
            GranulatorInstance() = default;

            /**
             * Init method for input granulation using a circular buffer
             * @param channelCount number of channels in the granulator
             * @param internalBufferSize  internal processing buffersize, defines the resolution of the timing opf the grains
             * @param VoiceCount number of voices in the voicepool
             * @param circularBuffer circular buffer used to process audio input
             * @param pitchTranslator translator table used to convert pitch in semitones to a factor
             * @param nodeManager nodemanager that the granulator runs on
             * @param errorState initialisation errors are logged here
             * @return true on success
             */
            bool init(int channelCount, int internalBufferSize, int VoiceCount, CircularBufferInstance& circularBuffer, SafePtr<TableTranslator<float>> pitchTranslator, NodeManager& nodeManager, utility::ErrorState& errorState);

            /**
             * Init method for sample buffer granulation using a memory buffer
             * @param channelCount number of channels in the granulator
             * @param internalBufferSize  internal processing buffersize, defines the resolution of the timing opf the grains
             * @param VoiceCount number of voices in the voicepool
             * @param buffer buffer containing sampled audio material
             * @param pitchTranslator translator table used to convert pitch in semitones to a factor
             * @param nodeManager nodemanager that the granulator runs on
             * @param errorState initialisation errors are logged here
             * @return true on success
             */
            bool init(int channelCount, int internalBufferSize, int VoiceCount, SafePtr<MultiSampleBuffer> buffer, SafePtr<TableTranslator<float>> pitchTranslator, NodeManager& nodeManager, utility::ErrorState& errorState);

            int getChannelCount() const override { return mChannels.size(); }
            OutputPin* getOutputForChannel(int channel) override;

            Channel& getChannel(int index) { return *mChannels[index]; }

            void setActiveChannelCount(int channelCount);
            void setActive(bool active);

        private:
            std::vector<SafeOwner<Channel>> mChannels;
            int mActiveChannelCount = 0;
            bool mActive = false;

            // The equal power table for amplitude calculation is shared
            std::unique_ptr<EqualPowerTable> mEqualPowerTable;

            // Helper methods
            bool initHelper(int channelCount, int internalBufferSize, int VoiceCount, CircularBufferInstance* circularBuffer, SafePtr<MultiSampleBuffer> buffer, SafePtr<TableTranslator<float>> pitchTranslator,NodeManager& nodeManager, utility::ErrorState& errorState);
        };


        /**
        * Manages the DSP for one single channel of the granulator.
        * All the DSP runs on a @NestedNodeManagerNode that can run on a lower internal buffersize for increased timing accuracy.
        */
        class NAPAPI GranulatorInstance::Channel
        {
        public:
            Channel() = default;

            /**
            * Initialize the granulator channel
            * @param channelIndex the index of the channel
            * @param internalBufferSize the internal buffersize of the granulator DSP
            * @param voiceCount the number of voices of the granulator per channel
            * @param voice resource for the DSP graph of a single voice
            * @param circularBuffer the circular buffer where the audio
            * @param pitchTranslator table that will be used to convert semitones difference to pitch factor
            * @param nodeManager main node manager of the DSP system
            * @param errorState error logging
            * @return true on success
            */
            bool init(int channelIndex, int internalBufferSize, int voiceCount, Voice& voice, SafePtr<CircularBufferNode> circularBuffer, SafePtr<TableTranslator<float>> pitchTranslator, NodeManager& nodeManager, utility::ErrorState& errorState);

            bool init(int channelIndex, int internalBufferSize, int voiceCount, Voice& voice, SafePtr<MultiSampleBuffer> buffer, SafePtr<TableTranslator<float>> pitchTranslator, NodeManager& nodeManager, utility::ErrorState& errorState);

            /**
            * Set the timing deviation in ms of the grains measured in whole times the average interval between the grains
            */
            void setIrregularity(float value) { mIrregularity = value; }

            /**
            * Set the deviation in the read position from the circular buffer in ms
            */
            void setDiffusion(float value) { mDiffusion = value; }

            /**
            * Set the shape of the amplitude envelope of the grains.
            */
            void setShape(GranulatorInstance::Shape shape) { mShape = shape; }

            /**
            * Sets the attack for expodec shaped grains/the decay of the rexpodec grains
            */
            void setAttackDecay(TimeValue value) { mAttackDecay = value; }

            /**
            * Sets the deviation of the transpose value in semitones
            */
            void setTransposeDeviation(float value) { mTransposeDeviation = value; }

            /**
            * Sets the average amplitude of the grains
            */
            void setAmplitude(float value) { mAmplitude = value; }

            /**
            * Sets the deviation of the amplitudes of the grains
            */
            void setAmplitudeDev(float value) { mAmplitudeDev = value; }

            /**
            * Sets the total number of channels in the granulator instance. Called by the granulator instance.
            */
            void setChannelCount(int value)
            {
                 mChannelCount = value;
                // reset phase.
                mCurrentPhase = mChannelIndex / float(mChannelCount);
            }

            /**
            * Sets wether the granulator is activated.
            */
            void setActive(bool active);

            /**
            * Sets the average transposition of the pitch in semitones.
            */
            void setTranspose(float value);

            /**
            * Sets the duration of the grains in ms.
            */
            void setDuration(TimeValue value);

            /**
            * Sets the density of the granulator: number of grains
            */
            void setDensity(float value);

            /**
             * Sets the read position within the memory buffer
             */
            void setPosition(double time);

            /**
            * Audio output signal
            */
            OutputPin* getOutput();

            /**
             * Change the source from which audio input is read to a circular buffer and set the mode accordingly to streaming input
             */
            void setSource(SafePtr<CircularBufferNode> buffer);

            /**
             * Change the buffer from which audio input is read to a memory buffer and set the mode accordingly
             */
            void setSource(SafePtr<MultiSampleBuffer> buffer);

        private:
            std::unique_ptr<NestedNodeManagerInstance> mNestedNodeManager = nullptr;
            std::unique_ptr<PolyphonicInstance> mPolyphonic = nullptr;
            std::unique_ptr<NodeObjectInstance<OutputNode>> mOutput = nullptr;

            SafePtr<CircularBufferNode> mCircularBuffer = nullptr;
            SafePtr<MultiSampleBuffer> mBuffer = nullptr;

            SafePtr<TableTranslator<float>> mPitchTranslator = nullptr;

            float mAmplitudeScaling = 1.f;
            int mChannelCount = 1;
            bool mActive = false;
            TimeValue mDuration = 500;
            float mIrregularity = 0;
            float mDiffusion = 0;
            float mDensity = 10;
            float mAmplitude = 1;
            float mAmplitudeDev = 0;
            Shape mShape = hanning;
            TimeValue mAttackDecay = 10;
            float mTranspose = 1.f;
            float mTransposeDeviation = 0;

            // Specific for memory buffer granulation
            double mPosition = 0; // in milliseconds
            float mSpeed = 0;

            // private:
        private:
            void updateAudio(DiscreteTimeValue sampleTime);
            void makeGrain();
            bool init(int channelIndex, int internalBufferSize, int voiceCount, Voice& voice, SafePtr<TableTranslator<float>> pitchTranslator, NodeManager& nodeManager, utility::ErrorState& errorState);

            double mCurrentPhase = 0;
            int mChannelIndex = 0;
            DiscreteTimeValue mLastSampleTime = 0;
            bool mAboutToSpawn = false;
            double mIrregularityOffset = 0.;
        };


    }

}
