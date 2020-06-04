#include "FastGainControl.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::FastGainControlNode)
    RTTI_PROPERTY("audioInput", &nap::audio::FastGainControlNode::audioInput, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("audioOutput", &nap::audio::FastGainControlNode::audioOutput, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_FUNCTION("setGain", &nap::audio::FastGainControlNode::setGain)
    RTTI_FUNCTION("getGain", &nap::audio::FastGainControlNode::getGain)
RTTI_END_CLASS

RTTI_DEFINE_CLASS(nap::audio::FastGainControl)



namespace nap
{
    
    namespace audio
    {
        
        void FastGainControlNode::process()
        {
            auto& outputBuffer = getOutputBuffer(audioOutput);
            auto& inputBuffer = *audioInput.pull();
            
            for (auto i = 0; i < outputBuffer.size(); ++i)
                outputBuffer[i] = inputBuffer[i] * mGain.getNextValue();
        }
        
        
        void FastGainControlNode::setGain(ControllerValue gain, TimeValue rampTime)
        {
            mGain.ramp(gain, rampTime * getNodeManager().getSamplesPerMillisecond());
        }
        
        
    }
    
}

