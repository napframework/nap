/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "audiofileutils.h"

// Std Includes
#include <stdint.h>
#include <iostream>

// Third party includes
#include <sndfile.h>

// Nap include
#include <utility/fileutils.h>
#include <utility/stringutils.h>

using namespace std;

namespace nap
{
	
	namespace audio
	{
		
		
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
			
			return true;
		}
		
		
		bool readAudioFile(const std::string& fileName, MultiSampleBuffer& output, float& outSampleRate, nap::utility::ErrorState& errorState)
		{
			return readLibSndFile(fileName, output, outSampleRate, errorState);
		}
		
		
	}
}