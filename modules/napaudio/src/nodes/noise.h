#pragma once

#include "audionode.h"

namespace nap {
    
    namespace audio {
        
        /**
         * White noise generator
         */
        class NAPAPI Noise : public Node
        {
        public:
            Noise(NodeManager& manager) : Node(manager) { }
        
            /**
             * Output signal containing the noise
             */
            OutputPin audioOutput = { this };
            
        private:
            void process() override;
        };
        
    }
}
