#include "gainnode.h"

namespace nap {
    
    namespace audio {
        
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
