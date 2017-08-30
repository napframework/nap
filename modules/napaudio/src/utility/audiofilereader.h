#pragma once

// std library includes
#include <string>

// nap includes
#include <audiotypes.h>

namespace nap {
    
    namespace audio {

        bool NAPAPI readAudioFile(const std::string& fileName, MultiSampleBuffer& buffer, float& sampleRate);
        
    }

}
