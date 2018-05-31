#include "gainnode.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::GainNode)
    RTTI_PROPERTY("inputs", &nap::audio::GainNode::inputs, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("audioOutput", &nap::audio::GainNode::audioOutput, nap::rtti::EPropertyMetaData::Embedded)
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
            auto gain = mGain.load();
            
            for (auto& inputBuffer : inputBuffers)
                if (inputBuffer == nullptr)
                {
                    for (auto i = 0; i < outputBuffer.size(); ++i)
                        outputBuffer[i] = 0;
                    return;
                }
            
            for (auto i = 0; i < outputBuffer.size(); ++i)
            {
                outputBuffer[i] = gain;
                for (auto& inputBuffer : inputBuffers)
                    outputBuffer[i] *= (*inputBuffer)[i];
            }
        }
        
    }
    
}
