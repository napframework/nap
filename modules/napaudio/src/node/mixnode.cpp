#include "mixnode.h"

namespace nap
{
    
    namespace audio
    {
        
        void MixNode::process()
        {
            auto& outputBuffer = getOutputBuffer(audioOutput);
            auto inputBuffers = inputs.pull();
            
            for (auto i = 0; i < outputBuffer.size(); ++i)
                outputBuffer[i] = 0;
            
            for (auto& inputBuffer : inputBuffers)
                if (inputBuffer)
                    for (auto i = 0; i < outputBuffer.size(); ++i)
                        outputBuffer[i] += (*inputBuffer)[i];
        }
        
    }
    
}

