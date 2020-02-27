#include <audio/node/oscillatornode.h>

#include <math.h>

// Rtti includes
#include <rtti/rtti.h>

// Audio includes
#include <audio/utility/audiofunctions.h>
#include <audio/utility/safeptr.h>
#include <audio/core/audionodemanager.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::OscillatorNode)
    RTTI_FUNCTION("setFrequency", &nap::audio::OscillatorNode::setFrequency)
    RTTI_FUNCTION("setAmplitude", &nap::audio::OscillatorNode::setAmplitude)
    RTTI_FUNCTION("setPhaseOffset", &nap::audio::OscillatorNode::setPhase)
    RTTI_PROPERTY("fmInput", &nap::audio::OscillatorNode::fmInput, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("audioOutput", &nap::audio::OscillatorNode::output, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
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


        OscillatorNode::OscillatorNode(NodeManager& manager) : Node(manager)
        {
            mFrequency.setStepCount(getNodeManager().getSamplesPerMillisecond());
            mAmplitude.setStepCount(getNodeManager().getSamplesPerMillisecond());
        }


        OscillatorNode::OscillatorNode(NodeManager& manager, SafePtr<WaveTable> wave) :
            Node(manager), mWave(wave)
        {
            mStep = mWave->getSize() / getNodeManager().getSampleRate();
            mFrequency.setStepCount(getNodeManager().getSamplesPerMillisecond());
            mAmplitude.setStepCount(getNodeManager().getSamplesPerMillisecond());
        }

        
        void OscillatorNode::process()
        {
            auto& outputBuffer = getOutputBuffer(output);
            SampleBuffer* fmInputBuffer = fmInput.pull();

            if (mWave == nullptr)
            {
                for (auto i = 0; i < getBufferSize(); ++i)
                    outputBuffer[i] = 0.f;
                return;
            }

            auto waveSize = mWave->getSize();
            auto step = mStep.load();
            auto phaseOffset = mPhaseOffset.load();
            
            for (auto i = 0; i < getBufferSize(); i++)
            {
                auto val = mAmplitude.getNextValue() * mWave->interpolate(mPhase + phaseOffset);   //   calculate new value, use wave as a lookup table
                if (fmInputBuffer)
                    mPhase += ((*fmInputBuffer)[i] + 1) * mFrequency.getNextValue() * step;      //   calculate new phase
                else
                    mPhase += mFrequency.getNextValue() * step;
                
                if (mPhase > waveSize)
                    mPhase -= waveSize;
                
                outputBuffer[i] = val;
            }
        }

        
        void OscillatorNode::setAmplitude(ControllerValue amplitude, TimeValue rampTime)
        {
            mAmplitude.setValue(amplitude);
        }
        
        
        void OscillatorNode::setPhase(ControllerValue phase)
        {
            mPhaseOffset = phase * mWave->getSize();
        }


        void OscillatorNode::setFrequency(SampleValue frequency, TimeValue rampTime)
        {
            mFrequency.setValue(frequency);
        }
        
        
        void OscillatorNode::setWave(SafePtr<WaveTable> wave)
        {
            mWave = wave;
            mStep = mWave->getSize() / getNodeManager().getSampleRate();
        }
        
        
        void OscillatorNode::sampleRateChanged(float sampleRate)
        {
            mStep = mWave->getSize() / sampleRate;
        }
    }
}
