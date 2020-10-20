#include "audiofileutils.h"

// Std Includes
#include <stdint.h>
#include <iostream>

// Third party includes
#include <sndfile.h>
#include <mpg123.h>

// Nap include
#include <nap/logger.h>
#include <utility/fileutils.h>
#include <utility/stringutils.h>

using namespace std;

namespace nap
{
	
	namespace audio
	{
		
		
		bool readMp3File(const std::string& fileName, MultiSampleBuffer& output, float& outSampleRate, nap::utility::ErrorState& errorState)
		{
			mpg123_handle *mpgHandle;
			std::vector<unsigned char> buffer;
			
			int error;
			long sampleRate;
			int channelCount;
			int encoding;
			size_t done;
			
			nap::Logger::info("Loading mp3 audio file: %s", fileName.c_str());
			
			// Acquire a mpg123 handle
			mpgHandle = mpg123_new(NULL, &error);
			if (mpgHandle == nullptr)
			{
				errorState.fail("Error loading mp3 while acquiring mpg123 handle.");
				return false;
			}
			
			// Set the format settings for the handle
			errorState.check(mpg123_format_none(mpgHandle) == MPG123_OK, "Error loading mp3 while setting format.");
			errorState.check(mpg123_format(mpgHandle, 44100, MPG123_MONO | MPG123_STEREO, MPG123_ENC_FLOAT_32) == MPG123_OK, "Error loading mp3 while setting format.");
			errorState.check(mpg123_format(mpgHandle, 48000, MPG123_MONO | MPG123_STEREO, MPG123_ENC_FLOAT_32) == MPG123_OK, "Error loading mp3 while setting format.");
			
			// Clean up when an error has occured
			if (!errorState.toString().empty())
			{
				mpg123_delete(mpgHandle);
				return false;
			}
			
			// Request the buffersize
			auto bufferSize = mpg123_outblock(mpgHandle);
			buffer.resize(bufferSize);
			
			// Open the file on the handle
			error = mpg123_open(mpgHandle, fileName.c_str());
			if (error != MPG123_OK)
			{
				errorState.fail("Mp3 file failed to open: %s", fileName.c_str());
				mpg123_delete(mpgHandle);
				return false;
			}
			
			// Request the format
			error = mpg123_getformat(mpgHandle, &sampleRate, &channelCount, &encoding);
			if (error != MPG123_OK)
			{
				errorState.fail("Failed to retrieve format.");
				mpg123_delete(mpgHandle);
				return false;
			}
			outSampleRate = sampleRate;
			
			// Request the size in bytes of one audio sample
			auto sampleSize = mpg123_encsize(encoding);
			if (sampleSize <= 0)
			{
				errorState.fail("Error requesting the sample size");
				mpg123_delete(mpgHandle);
				return false;
			}
			
			auto channelOffset = output.getSize();
			output.resize(channelOffset + channelCount, 0);
			
			auto formattedBuffer = reinterpret_cast<float*>(buffer.data());
			
			while (mpg123_read(mpgHandle, buffer.data(), bufferSize, &done) == MPG123_OK)
			{
				auto frameCount = done / (sampleSize * channelCount);
				auto offset = output.getSize();
				output.resize(channelOffset + channelCount, output.getSize() + frameCount);
				auto i = 0;
				for (auto frame = 0; frame < frameCount; ++frame)
					for (auto channel = channelOffset; channel < channelOffset + channelCount; ++channel)
					{
						output[channel][offset + frame] = formattedBuffer[i];
						i++;
					}
			}
			
			mpg123_close(mpgHandle);
			mpg123_delete(mpgHandle);
			
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
			auto channelOffset = output.getChannelCount();
			output.resize(channelOffset + info.channels, info.frames);
			int i = 0;
			for (auto frame = 0; frame < info.frames; ++frame)
				for (auto channel = channelOffset; channel < channelOffset + info.channels; ++channel)
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
			if (utility::toLower(utility::getFileExtension(fileName)) == "mp3")
				return readMp3File(fileName, output, outSampleRate, errorState);
			
			return readLibSndFile(fileName, output, outSampleRate, errorState);
		}
		
		
	}
}