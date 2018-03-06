#include "delaynode.h"

#include <audio/utility/audiofunctions.h>
#include <audio/core/audionodemanager.h>

namespace nap
{
    
    namespace audio
    {
        
        void DelayNode::setTime(TimeValue value, TimeValue rampTime)
        {
            mTimeRamper.ramp(int(value * getNodeManager().getSamplesPerMillisecond()), rampTime * getNodeManager().getSamplesPerMillisecond());
        }
        
        
        void DelayNode::setDryWet(ControllerValue value, TimeValue rampTime)
        {
            mDryWetRamper.ramp(value, rampTime * getNodeManager().getSamplesPerMillisecond());
        }


        void DelayNode::process()
        {
            auto& inputBuffer = *input.pull();
            auto& outputBuffer = getOutputBuffer(output);
            SampleValue delayedSample = 0;
            
            for (auto i = 0; i < outputBuffer.size(); ++i)
            {
                if (mTimeRamper.isRamping())
                    delayedSample = mDelay.readInterpolating(mTime);
                else
                    delayedSample = mDelay.read(mTime);
                
                mDelay.write(inputBuffer[i] + delayedSample * mFeedback);
                outputBuffer[i] = lerp(inputBuffer[i], delayedSample, mDryWet);
                
                mTimeRamper.step();
                mDryWetRamper.step();
            }
        }

    }
    
}
