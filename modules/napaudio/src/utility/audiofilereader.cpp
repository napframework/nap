#include "audiofilereader.h"

#include <string>

// third party includes
#include <sndfile.h>

// nap include
#include <nap/logger.h>

using namespace std;

namespace nap {
    
    namespace audio {
        
        
        bool readAudioFile(const std::string& fileName, MultiSampleBuffer& output, float& outSampleRate)
        {
            SF_INFO info;
            
            // try to open sound file
            auto sndFile = sf_open(fileName.c_str(), SFM_READ, &info);
            if (sf_error(sndFile) != SF_ERR_NO_ERROR)
            {
                nap::Logger::fatal(sf_strerror(sndFile));
                return false;
            }
            
            // allocare a float buffer to read into by libsndfile
            float* readBuffer;
            readBuffer = new float[info.channels * info.frames];
            
            // do the reading
            sf_readf_float(sndFile, readBuffer, info.frames);
            if (sf_error(sndFile) != SF_ERR_NO_ERROR)
            {
                nap::Logger::fatal(sf_strerror(sndFile));
                return false;
            }
            
            // copy the read buffer into the output, also performs de-interleaving of the data into separate channels
            output.resize(info.channels, info.frames);
            auto i = 0;
            for (auto frame = 0; frame < info.frames; ++frame)
                for (auto channel = 0; channel < info.channels; ++channel)
                {
                    output[channel][frame] = readBuffer[i];
                    i++;
                }
            
            // set the samplerate
            outSampleRate = info.samplerate;
            
            sf_close(sndFile);
            
            
            return true;
        }

    }
}

