#include "FastDelay.h"

#include <audio/utility/audiofunctions.h>
#include <audio/core/audionodemanager.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::FastDelayNode)
    RTTI_PROPERTY("input", &nap::audio::FastDelayNode::audioInput, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("output", &nap::audio::FastDelayNode::output, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_FUNCTION("setTime", &nap::audio::FastDelayNode::setTime)
    RTTI_FUNCTION("setFeedback", &nap::audio::FastDelayNode::setFeedback)
    RTTI_FUNCTION("getTime", &nap::audio::FastDelayNode::getTime)
    RTTI_FUNCTION("getFeedback", &nap::audio::FastDelayNode::getFeedback)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::audio::FastDelay)
    RTTI_PROPERTY("DelayLineSize", &nap::audio::FastDelay::mDelayLineSize, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        FastDelayNode::FastDelayNode(NodeManager& manager, int delayLineSize) : Node(manager), mDelay(delayLineSize)
        {
            mTime.setStepCount(manager.getSamplesPerMillisecond());
        }
        
        
        void FastDelayNode::setTime(TimeValue value, TimeValue rampTime)
        {
            mTime.setStepCount(rampTime * getNodeManager().getSamplesPerMillisecond());
            mTime.setValue(int(value * getNodeManager().getSamplesPerMillisecond()));
        }
        
        void FastDelayNode::setDelayLineSize(int delayLineSize){
            mDelay = Delay(delayLineSize);
        }
        
        
        void FastDelayNode::process()
        {
            auto& inputBuffer = *audioInput.pull();
            auto& outputBuffer = getOutputBuffer(output);
            SampleValue delayedSample = 0;

            mTime.update();
            if (mPositive)
            {
                for (auto i = 0; i < outputBuffer.size(); ++i)
                {
                    if (mTime.isRamping())
                        delayedSample = mDelay.readInterpolating(mTime.getNextValue());
                    else
                        delayedSample = mDelay.read(mTime.getNextValue());
                    
                    mDelay.write(inputBuffer[i] + delayedSample * mFeedback);
                    outputBuffer[i] = delayedSample;
                }
            }
            else {
                for (auto i = 0; i < outputBuffer.size(); ++i)
                {
                    if (mTime.isRamping())
                        delayedSample = mDelay.readInterpolating(mTime.getNextValue());
                    else
                        delayedSample = mDelay.read(mTime.getNextValue());
                    
                    mDelay.write(inputBuffer[i] + delayedSample * mFeedback);
                    outputBuffer[i] = -delayedSample;
                }
            }
        }
        
        
        void FastDelayNode::clear()
        {
            mDelay.clear();
        }


    }
    
}
