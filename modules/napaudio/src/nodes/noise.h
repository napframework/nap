#pragma once

#include "audionode.h"

namespace nap {
    
    namespace audio {
        
        class Noise : public AudioNode {
        public:
            Noise(AudioService& service) : AudioNode(service) { }
        
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
