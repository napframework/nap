#include "delaynode.h"

#include <audio/utility/audiofunctions.h>
#include <audio/core/audionodemanager.h>

namespace nap
{
    
    namespace audio
    {
        
        void DelayNode::setTime(TimeValue value, TimeValue rampTime)
        {
            mTime.ramp(int(value * getNodeManager().getSamplesPerMillisecond()), rampTime * getNodeManager().getSamplesPerMillisecond());
        }
        
        
        void DelayNode::setDryWet(ControllerValue value, TimeValue rampTime)
        {
            mDryWet.ramp(value, rampTime * getNodeManager().getSamplesPerMillisecond());
        }


        void DelayNode::process()
        {
            auto& inputBuffer = *input.pull();
            auto& outputBuffer = getOutputBuffer(output);
            auto feedback = mFeedback.load();
            SampleValue delayedSample = 0;
            
            for (auto i = 0; i < outputBuffer.size(); ++i)
            {
                if (mTime.isRamping())
                    delayedSample = mDelay.readInterpolating(mTime.getValue());
                else
                    delayedSample = mDelay.read(mTime.getValue());
                
                mDelay.write(inputBuffer[i] + delayedSample * feedback);
                outputBuffer[i] = lerp(inputBuffer[i], delayedSample, mDryWet.getValue());
                
                mTime.step();
                mDryWet.step();
            }
        }

    }
    
}
