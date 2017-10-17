#pragma once

#include <core/audionode.h>

namespace nap
{
    namespace audio
    {
        
        /**
         * A wavetable that can be used as waveform data for an oscillator.
         * Contains a buffer with one cycle of samples for a periodic waveform.
         */
        class WaveTable
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
        class OscillatorNode : public Node
        {
            RTTI_ENABLE(Node)
            
        public:
            /**
             * Constructor takes the waveform of the oscillator
             */
            OscillatorNode(NodeManager& aManager, WaveTable& aWave);
            
            /**
             * Set the frequency in Hz
             */
            void setFrequency(SampleValue frequency);
            
            /**
             * Set the amplitude of the generated wave
             */
            void setAmplitude(ControllerValue amplitude);
            
            /**
             * Sets the phase of the oscillator as a value between 0 and 1
             */
            void setPhase(ControllerValue phaseOffset);
            
            /**
             * Set a new waveform for the oscillator
             */
            void setWave(WaveTable& aWave);
            
            ControllerValue getFrequency() const { return mFrequency; }
            ControllerValue getAmplitude() const { return mAmplitude; }
            ControllerValue getPhase() const { return mPhaseOffset / float(mWave.getSize()); }

            InputPin fmInput;
            OutputPin output = { this };

        private:
            void process() override;
            void sampleRateChanged(float sampleRate) override;

            WaveTable& mWave;

            ControllerValue mFrequency = 0;
            ControllerValue mAmplitude = 1.f;
            ControllerValue mStep = 0;
            ControllerValue mPhase = 0;
            ControllerValue mPhaseInc = 0;
            ControllerValue mPhaseOffset = 0;
        };
    }
}

