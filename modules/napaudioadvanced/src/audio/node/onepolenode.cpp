#include "onepolenode.h"

#include <audio/core/audionodemanager.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::OnePoleLowPassNode)
    RTTI_FUNCTION("setCutoffFrequency", &nap::audio::OnePoleLowPassNode::setCutoffFrequency)
    RTTI_FUNCTION("getCutoffFrequency", &nap::audio::OnePoleLowPassNode::getCutoffFrequency)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::OnePoleHighPassNode)
    RTTI_FUNCTION("setCutoffFrequency", &nap::audio::OnePoleHighPassNode::setCutoffFrequency)
    RTTI_FUNCTION("getCutoffFrequency", &nap::audio::OnePoleHighPassNode::getCutoffFrequency)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        // --- Low pass --- //
        
        void OnePoleLowPassNode::process()
        {
            auto& outputBuffer = getOutputBuffer(output);
            auto& inputBuffer = *input.pull();
            
            for (auto i = 0; i < outputBuffer.size(); ++i)
            {
                outputBuffer[i] = a0.getNextValue() * inputBuffer[i] + b1.getNextValue() * mTemp;
                mTemp = outputBuffer[i];
            }
        }
        
        
        void OnePoleLowPassNode::setCutoffFrequency(ControllerValue cutoff)
        {
            mCutOff = cutoff;
            auto c = mCutOff / getNodeManager().getSampleRate();
            auto x = pow(M_E, -2 * M_PI * c);
            a0.setValue(1.0 - x);
            b1.setValue(x);
        }
        
        
        void OnePoleLowPassNode::setRampTime(TimeValue value)
        {
            int stepCount = value * getNodeManager().getSamplesPerMillisecond();
            a0.setStepCount(stepCount);
            b1.setStepCount(stepCount);
        }

        
        // --- High pass --- //
        
        void OnePoleHighPassNode::process()
        {
            auto& outputBuffer = getOutputBuffer(output);
            auto& inputBuffer = *input.pull();
            
            for (auto i = 0; i < outputBuffer.size(); ++i)
            {
                outputBuffer[i] = a0.getNextValue() * inputBuffer[i] + a1.getNextValue() * mTemp1 + b1.getNextValue() * mTemp2;
                mTemp1 = inputBuffer[i];
                mTemp2 = outputBuffer[i];
            }
        }
        
        
        void OnePoleHighPassNode::setCutoffFrequency(ControllerValue cutoff)
        {
            mCutOff = cutoff;
            auto c = mCutOff / getNodeManager().getSampleRate();
            auto x = pow(M_E, -2 * M_PI * c);
            a0.setValue((1.0 + x) / 2.0);
            a1.setValue(-(1.0 + x) / 2.0);
            b1.setValue(x);
        }
        
        
        void OnePoleHighPassNode::setRampTime(TimeValue value)
        {
            int stepCount = value * getNodeManager().getSamplesPerMillisecond();
            a0.setStepCount(stepCount);
            a1.setStepCount(stepCount);
            b1.setStepCount(stepCount);
        }


    }
    
}

