#pragma once

// std library includes
#include <string>

// nap includes
#include <audiotypes.h>

namespace nap {
    
    namespace audio {

        /**
         * Utility to read an audio file from disk
         * @param fileName: the absolute path to the file
         * @param buffer: the buffer to read the file into
         * @param sampleRate: this will contain the sample rate of the audio file after success
         * @return: true on success
         */
        bool NAPAPI readAudioFile(const std::string& fileName, MultiSampleBuffer& buffer, float& sampleRate);
        
    }

}
