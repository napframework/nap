#pragma once

#include "audionode.h"

namespace nap {
    
    namespace audio {
        
        class Noise : public AudioNode {
        public:
            Noise(AudioNodeManager& manager) : AudioNode(manager) { }
        
            AudioOutput audioOutput = { this, &Noise::calculate };
            
        private:
            void calculate(SampleBuffer& buffer)
            {
                for (auto i = 0; i < buffer.size(); ++i)
                    buffer[i] = rand() / float(RAND_MAX);
            }
        };
        
    }
}
