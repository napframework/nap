#pragma once

// Std includes
#include <atomic>

#include <audio/core/audionode.h>
#include <audio/utility/linearsmoothedvalue.h>
#include <audio/utility/safeptr.h>

namespace nap
{
    namespace audio
    {
        
        /**
         * A wavetable that can be used as waveform data for an oscillator.
         * Contains a buffer with one cycle of samples for a periodic waveform.
         */
        class NAPAPI WaveTable
        {
        public:
            /**
             * Constructor takes the size of the waveform buffer.
             */
            WaveTable(long size);
            
            /**
             * Constructor takes the size of the waveform buffer
             */
            WaveTable(long size, SampleBuffer& spectrum, int stepSize);
            
            /**
             * Normalize the waveform so the "loudest" sample has amplitude 1.f
             */
            void normalize();
            
            /**
             * Subscript operator to access the waveform's samples
             */
            inline SampleValue& operator[](long index);
            inline SampleValue operator[](long index) const;
            
            /**
             * Read from the waveform at a certain index between 0 and @getSize()
             */
            inline SampleValue interpolate(double index) const;
            
            /**
             * Returns the size of the waveform buffer
             */
            long getSize() const { return mData.size(); }
            
        private:
            SampleBuffer mData;
        };

        
        /**
         * Oscillator that generates an audio signal from a periodic waveform and a frequency
         */
        class NAPAPI OscillatorNode : public Node
        {
            RTTI_ENABLE(Node)
            
        public:
            OscillatorNode(NodeManager& manager);

            /**
             * Constructor takes the waveform of the oscillator.
             */
            OscillatorNode(NodeManager& aManager, SafePtr<WaveTable> wave);

            /**
             * Set the frequency in Hz
             */
            void setFrequency(ControllerValue frequency, TimeValue rampTime = 0);
            
            /**
             * Set the amplitude of the generated wave
             */
            void setAmplitude(ControllerValue amplitude, TimeValue rampTime = 0);
            
            /**
             * Sets the phase of the oscillator as a value between 0 and 1
             */
            void setPhase(ControllerValue phaseOffset);
            
            /**
             * Set the waveform for the oscillator.
             * Has to be called after construction and before usage.
             */
            void setWave(SafePtr<WaveTable> aWave);
            
            InputPin fmInput = { this }; ///< Input pin to control frequency modulation.
            OutputPin output = { this }; ///< Audio output pin.

        private:
            void process() override;
            void sampleRateChanged(float sampleRate) override;

            SafePtr<WaveTable> mWave = nullptr;

            LinearSmoothedValue<ControllerValue> mFrequency = { 440, 44 };
            LinearSmoothedValue<ControllerValue> mAmplitude = { 1.f, 44 };
            
            std::atomic<ControllerValue> mStep = { 0 };
            std::atomic<ControllerValue> mPhaseOffset = { 0 };
            
            ControllerValue mPhase = 0;
        };
    }
}

