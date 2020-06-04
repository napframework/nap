#pragma once

// Audio includes
#include <audio/core/audionode.h>
#include <audio/utility/linearsmoothedvalue.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Node that doesn't do anything at all.
         */
        class NAPAPI PassThroughNode : public Node
        {
        public:
            PassThroughNode(NodeManager& manager) : Node(manager) { }
            
            InputPin audioInput = { this };
            
            OutputPin audioOutput = { this };
            
            void process() override {
                auto& outputBuffer = getOutputBuffer(audioOutput);
                auto& inputBuffer = *audioInput.pull();
                outputBuffer = inputBuffer;
            }

        };
        
    }
}

