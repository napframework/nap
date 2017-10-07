#include "audiofilereader.h"

// Std Includes
#include <stdint.h>
#include <iostream>

// Third party includes
#include <sndfile.h>
#include <mpg123.h>

// Nap include
#include <nap/logger.h>
#include <nap/fileutils.h>
#include <utility/stringutils.h>

using namespace std;

namespace nap {
    
    namespace audio {
        
        
        bool readMp3File(const std::string& fileName, MultiSampleBuffer& output, float& outSampleRate, nap::utility::ErrorState& errorState)
        {
            mpg123_handle *mpgHandle;
            std::vector<unsigned char> buffer;
            
            int error;
            long sampleRate;
            int channelCount;
            int encoding;
            size_t done;
            
            mpg123_init();
            
            mpgHandle = mpg123_new(NULL, &error);
            mpg123_format_none(mpgHandle);
            mpg123_format(mpgHandle, 44100, MPG123_MONO | MPG123_STEREO, MPG123_ENC_FLOAT_32);
            mpg123_format(mpgHandle, 48000, MPG123_MONO | MPG123_STEREO, MPG123_ENC_FLOAT_32);
            mpg123_format(mpgHandle, 88200, MPG123_MONO | MPG123_STEREO, MPG123_ENC_FLOAT_32);
            mpg123_format(mpgHandle, 96000, MPG123_MONO | MPG123_STEREO, MPG123_ENC_FLOAT_32);

            auto bufferSize = mpg123_outblock(mpgHandle);
            buffer.resize(bufferSize);
            
            mpg123_open(mpgHandle, fileName.c_str());
            mpg123_getformat(mpgHandle, &sampleRate, &channelCount, &encoding);
            auto sampleSize = mpg123_encsize(encoding);
            
            output.clear();
            output.resize(channelCount, 0);
            
            auto formattedBuffer = reinterpret_cast<float*>(buffer.data());
            
            while (mpg123_read(mpgHandle, buffer.data(), bufferSize, &done) == MPG123_OK)
            {
                auto frameCount = done / (sampleSize * channelCount);
                auto offset = output.getSize();
                output.resize(channelCount, output.getSize() + frameCount);
                auto i = 0;
                for (auto frame = 0; frame < frameCount; ++frame)
                    for (auto channel = 0; channel < channelCount; ++channel)
                    {
                        output[channel][offset + frame] = formattedBuffer[i];
                        i++;
                    }
            }
            
            mpg123_close(mpgHandle);
            mpg123_delete(mpgHandle);
            mpg123_exit();
            
            nap::Logger::info("Loaded mp3 audio file: %s", fileName.c_str());
            
            return true;
        }
        
        
        bool readLibSndFile(const std::string& fileName, MultiSampleBuffer& output, float& outSampleRate, nap::utility::ErrorState& errorState)
        {
            SF_INFO info;
            
            // try to open sound file
            auto sndFile = sf_open(fileName.c_str(), SFM_READ, &info);
            if (sf_error(sndFile) != SF_ERR_NO_ERROR)
            {
                errorState.fail("Failed to load audio file %s: %s", fileName.c_str(), sf_strerror(sndFile));
                return false;
            }
            
            // allocare a float buffer to read into by libsndfile
            std::vector<SampleValue> readBuffer;
            readBuffer.resize(info.channels * info.frames);
            
            // do the reading
            sf_readf_float(sndFile, readBuffer.data(), info.frames);
            if (sf_error(sndFile) != SF_ERR_NO_ERROR)
            {
                errorState.fail("Failed to load audio file %s: %s", fileName.c_str(), sf_strerror(sndFile));
                return false;
            }
            
            // copy the read buffer into the output, also performs de-interleaving of the data into separate channels
            output.resize(info.channels, info.frames);
            int i = 0;
            for (auto frame = 0; frame < info.frames; ++frame)
                for (auto channel = 0; channel < info.channels; ++channel)
                {
                    output[channel][frame] = readBuffer[i];
                    i++;
                }
            
            // set the samplerate
            outSampleRate = info.samplerate;
            
			// cleanup
            sf_close(sndFile);
            
            nap::Logger::info("Loaded audio file: %s", fileName.c_str());
            
            return true;
        }
        
        
        bool readAudioFile(const std::string& fileName, MultiSampleBuffer& output, float& outSampleRate, nap::utility::ErrorState& errorState)
        {
            if (utility::gToLower(getFileExtension(fileName)) == "mp3")
                return readMp3File(fileName, output, outSampleRate, errorState);
            
            return readLibSndFile(fileName, output, outSampleRate, errorState);
        }


    }
}

