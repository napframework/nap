#pragma once

#include "audionode.h"

namespace nap {
    
    namespace audio {
        
        /**
         * White noise generator
         */
        class NAPAPI Noise : public AudioNode {
        public:
            Noise(AudioNodeManager& manager) : AudioNode(manager) { }
        
            /**
             * Output signal containing the noise
             */
            AudioOutput audioOutput = { this, &Noise::calculate };
            
        private:
            void calculate(SampleBuffer& buffer);
        };
        
    }
}
