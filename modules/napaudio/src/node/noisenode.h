#pragma once

#include <core/audionode.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * White noise generator
         */
        class NAPAPI NoiseNode : public Node
        {
        public:
            NoiseNode(NodeManager& manager) : Node(manager) { }
        
            /**
             * Output signal containing the noise
             */
            OutputPin audioOutput = { this };
            
        private:
            void process() override;
        };
        
    }
}
