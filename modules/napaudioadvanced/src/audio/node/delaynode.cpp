#include "delaynode.h"

#include <audio/utility/audiofunctions.h>
#include <audio/core/audionodemanager.h>

namespace nap
{
    
    namespace audio
    {
        
        DelayNode::DelayNode(NodeManager& manager, int delayLineSize) : Node(manager), mDelay(delayLineSize)
        {
            mTime.setStepCount(manager.getSamplesPerMillisecond());
            mDryWet.setStepCount(manager.getSamplesPerMillisecond());
        }
        
        
        void DelayNode::setTime(TimeValue value, TimeValue rampTime)
        {
            mTime.setValue(int(value * getNodeManager().getSamplesPerMillisecond()));
        }
        
        
        void DelayNode::setDryWet(ControllerValue value, TimeValue rampTime)
        {
            mDryWet.setValue(value);
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
                    delayedSample = mDelay.readInterpolating(mTime.getNextValue());
                else
                    delayedSample = mDelay.read(mTime.getNextValue());
                
                mDelay.write(inputBuffer[i] + delayedSample * feedback);
                outputBuffer[i] = lerp(inputBuffer[i], delayedSample, mDryWet.getNextValue());
            }
        }

    }
    
}
