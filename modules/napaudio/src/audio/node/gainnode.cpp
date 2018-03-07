#include "gainnode.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::GainNode)
    RTTI_CONSTRUCTOR(nap::audio::NodeManager&)
    RTTI_FUNCTION("setGain", &nap::audio::GainNode::setGain)
    RTTI_FUNCTION("getGain", &nap::audio::GainNode::getGain)
RTTI_END_CLASS


namespace nap
{
    
    namespace audio
    {
        
        void GainNode::process()
        {
            auto& outputBuffer = getOutputBuffer(audioOutput);
            auto inputBuffers = inputs.pull();
            
            for (auto& inputBuffer : inputBuffers)
                if (inputBuffer == nullptr)
                {
                    for (auto i = 0; i < outputBuffer.size(); ++i)
                        outputBuffer[i] = 0;
                    return;
                }
            
            for (auto i = 0; i < outputBuffer.size(); ++i)
            {
                outputBuffer[i] = mGain;
                for (auto& inputBuffer : inputBuffers)
                    outputBuffer[i] *= (*inputBuffer)[i];
            }
        }
        
    }
    
}
