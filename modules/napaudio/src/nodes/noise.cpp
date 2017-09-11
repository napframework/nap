#include "noise.h"

// Std includes
#include <stdlib.h>

namespace nap {
    
    namespace audio {
        
        void Noise::process()
        {
            auto& buffer = getOutputBuffer(audioOutput);
            for (auto i = 0; i < buffer.size(); ++i)
                buffer[i] = rand() / float(RAND_MAX);
        }
        
    }
    
}
