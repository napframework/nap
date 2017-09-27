#pragma once

#include <node/audionode.h>

namespace nap {
    
    namespace audio {
        
        /**
         * Node to scale an audio signal
         */
        class NAPAPI MixNode : public Node
        {
        public:
            MixNode(NodeManager& manager) : Node(manager) { }
            
            /**
             * The input to be scaled
             */
            MultiInputPin inputs;
            
            /**
             * Outputs the scaled signal
             */
            OutputPin audioOutput = { this };
            
        private:
            /**
             * Calculate the output, perform the scaling
             */
            void process() override;
        };
        
    }
}

