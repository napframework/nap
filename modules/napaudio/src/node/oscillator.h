#pragma once

#include <node/audionode.h>

namespace nap
{
    namespace audio
    {
        class WaveTable
        {
        public:
            WaveTable(long size);
            WaveTable(long size, SampleBuffer& spectrum, int stepSize);
            
            void normalize();
            
            inline SampleValue& operator[](long index);
            inline SampleValue operator[](long index) const;
            
            inline SampleValue interpolate(double index) const;
            
            long getSize() const { return mData.size(); }
            
        private:
            SampleBuffer mData;
        };

        
        class Oscillator : public Node
        {
        public:
            Oscillator(NodeManager& aManager, WaveTable& aWave);
            
            void setFrequency(SampleValue frequency);
            ControllerValue getFrequency() { return mFrequency; }
            void setAmplitude(ControllerValue amplitude);
            void setPhase(SampleValue phase) { mPhase = phase; }
            void setPhaseOffset(SampleValue phaseOffset) { mPhaseOffset = phaseOffset; };
            void setWave(WaveTable& aWave);
            
            InputPin fmInput;
            OutputPin output = { this };

        private:
            void process() override;
            void sampleRateChanged(float sampleRate) override;

            WaveTable& mWave;

            ControllerValue mFrequency = 0;
            ControllerValue mAmplitude = 1.;
            ControllerValue mStep = 0;
            ControllerValue mPhase = 0;
            ControllerValue mPhaseInc = 0;
            ControllerValue mPhaseOffset = 0;
            
        };
    }
}

