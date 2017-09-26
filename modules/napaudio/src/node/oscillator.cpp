#include <node/oscillator.h>

#include <math.h>

// Audio includes
#include <utility/audiofunctions.h>
#include <node/audionodemanager.h>

namespace nap {
    namespace audio {
        
// --- Wavetable --- //
        
        WaveTable::WaveTable(long size)
        {
            mData.resize(size);
            auto step = M_PI * 2 / size;
            for(int i = 0; i < size; i++)
                mData[i] = sin(i * step);
        }


        WaveTable::WaveTable(long size, SampleBuffer& spectrum, int stepSize)
        {
            mData.resize(size);
            auto step = M_PI * 2 / size;
            
            for(int i = 0; i < size; i++){
                mData[i] = 0.0;
                for(int j = 0; j < spectrum.size(); j+=stepSize)
                {
                    mData[i] += spectrum[j] * sin(i * j * step);
                }
            }
            
            normalize();
        }

        
        void WaveTable::normalize()
        {
            SampleValue max, min;
            max = min = mData[0];
            
            for(int i = 0; i < mData.size(); i++){
                if(mData[i] > max) max = mData[i];
                if(mData[i] < min) min = mData[i];
            }
            
            if(max < -min) max = -min;
            for(int i = 0; i < mData.size(); i++)
                mData[i] /= max;
        }

        
        SampleValue& WaveTable::operator[](long index)
        {
            return mData[wrap(index, mData.size())];
        }
        
        
        SampleValue WaveTable::operator[](long index) const
        {
            return mData[wrap(index, mData.size())];
        }

        
        SampleValue WaveTable::interpolate(double index) const
        {
            int floor = index;
            SampleValue frac = index - floor;
            
            auto v1 = mData[wrap(floor, mData.size())];
            auto v2 = mData[wrap(floor + 1, mData.size())];

            return lerp(v1, v2, frac);
        }

        
// --- Oscillator --- //

        Oscillator::Oscillator(NodeManager& manager, WaveTable& aWave) :
            Node(manager),
            mWave(aWave)
        {
            mStep = mWave.getSize() / getNodeManager().getSampleRate();
            setFrequency(220);
        }

        
        void Oscillator::process()
        {
            auto& outputBuffer = getOutputBuffer(output);
            SampleBuffer* fmInputBuffer = fmInput.pull();
            
            auto waveSize = mWave.getSize();
            
            for (auto i = 0; i < getBufferSize(); i++)
            {
                auto val = mAmplitude * mWave.interpolate(mPhase + mPhaseOffset);   //   calculate new value, use wave as a lookup table
                if (fmInputBuffer)
                    mPhase += ((*fmInputBuffer)[i] + 1) * mFrequency * mStep;      //   calculate new phase
                else
                    mPhase += mPhaseInc;
                
                mPhase = wrap(mPhase, waveSize);
                
                outputBuffer[i] = val;
            }
        }

        void Oscillator::setAmplitude(SampleValue amplitude)
        {
            mAmplitude = amplitude;
        }

        void Oscillator::setFrequency(SampleValue frequency)
        {
            mFrequency = frequency;
            mPhaseInc = mFrequency * mStep;
        }
        
        void Oscillator::setWave(WaveTable& wave)
        {
            mWave = wave;
            mStep = mWave.getSize() / getNodeManager().getSampleRate();
            mPhaseInc = mFrequency * mStep;
        }
        
        void Oscillator::sampleRateChanged(float sampleRate)
        {
            mStep = mWave.getSize() / sampleRate;
        }
    }
}
