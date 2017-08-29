#pragma once

#include <stdio.h>
#include <audiotypes.h>

namespace nap {
    
    namespace audio {

        bool NAPAPI readAudioFile(const std::string& fileName, MultiSampleBuffer& buffer, float& sampleRate);
        
    }

}
