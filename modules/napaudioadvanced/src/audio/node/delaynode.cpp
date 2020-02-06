#include "delaynode.h"

#include <audio/utility/audiofunctions.h>
#include <audio/core/audionodemanager.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::DelayNode)
    RTTI_PROPERTY("input", &nap::audio::DelayNode::input, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("output", &nap::audio::DelayNode::output, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_FUNCTION("setTime", &nap::audio::DelayNode::setTime)
    RTTI_FUNCTION("setDryWet", &nap::audio::DelayNode::setDryWet)
    RTTI_FUNCTION("setFeedback", &nap::audio::DelayNode::setFeedback)
    RTTI_FUNCTION("getTime", &nap::audio::DelayNode::getTime)
    RTTI_FUNCTION("getDryWet", &nap::audio::DelayNode::getDryWet)
    RTTI_FUNCTION("getFeedback", &nap::audio::DelayNode::getFeedback)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        DelayNode::DelayNode(NodeManager& manager, int delayLineSize) : Node(manager), mDelay(delayLineSize)
        {
            mTime.setStepCount(manager.getSamplesPerMillisecond() * 10);
            mDryWet.setStepCount(manager.getSamplesPerMillisecond() * 10);
        }
        
        
        void DelayNode::setTime(TimeValue value, TimeValue rampTime)
        {
//            mTime.setStepCount(rampTime * getNodeManager().getSamplesPerMillisecond());
            mTime.setValue(value * getNodeManager().getSamplesPerMillisecond());
        }
        
        
        void DelayNode::setDryWet(ControllerValue value, TimeValue rampTime)
        {
//            mDryWet.setStepCount(rampTime * getNodeManager().getSamplesPerMillisecond());
            mDryWet.setValue(value);
        }


        void DelayNode::process()
        {
            auto inputBuffer = input.pull();
            auto& outputBuffer = getOutputBuffer(output);
            auto feedback = mFeedback.load();
            SampleValue delayedSample = 0;
            
            if (inputBuffer)
            {
                for (auto i = 0; i < outputBuffer.size(); ++i)
                {
                    if (mTime.isRamping())
                        delayedSample = mDelay.readInterpolating(mTime.getNextValue());
                    else
                        delayedSample = mDelay.read(mTime.getNextValue());
                    
                    mDelay.write((*inputBuffer)[i] + delayedSample * feedback);
                    outputBuffer[i] = lerp((*inputBuffer)[i], delayedSample, mDryWet.getNextValue());
                }
            }
            else {
                // process with 0 input
                for (auto i = 0; i < outputBuffer.size(); ++i)
                {
                    if (mTime.isRamping())
                        delayedSample = mDelay.readInterpolating(mTime.getNextValue());
                    else
                        delayedSample = mDelay.read(mTime.getNextValue());
                    
                    mDelay.write(0.f + delayedSample * feedback);
                    outputBuffer[i] = lerp(0.f, delayedSample, mDryWet.getNextValue());
                }
            }
        }

    }
    
}
