/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "audiofileutils.h"

// Std Includes
#include <stdint.h>
#include <iostream>
#include <utility>

// Third party includes
#include <sndfile.h>
// #include <mpg123.h>
#include <samplerate.h>

// Nap include
#include <utility/fileutils.h>
#include <utility/stringutils.h>
#include <mathutils.h>

#include <nap/logger.h>


RTTI_BEGIN_ENUM(nap::audio::EResampleMode)
	RTTI_ENUM_VALUE(nap::audio::EResampleMode::SincBestQuality, "Sinc Best Quality"),
	RTTI_ENUM_VALUE(nap::audio::EResampleMode::SincMediumQuality, "Sinc Medium Quality"),
	RTTI_ENUM_VALUE(nap::audio::EResampleMode::SincFastest, "Sinc Fastest"),
	RTTI_ENUM_VALUE(nap::audio::EResampleMode::ZeroOrderHold, "Zero Order Hold"),
	RTTI_ENUM_VALUE(nap::audio::EResampleMode::Linear, "Linear")
RTTI_END_ENUM



using namespace std;

namespace nap
{
	
	namespace audio
	{
		bool readAudioFile(const std::string& fileName, MultiSampleBuffer& output, float& outSampleRate, nap::utility::ErrorState& errorState)
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


		bool NAPAPI resampleSampleBuffer(MultiSampleBuffer& buffer, float sourceSampleRate, float destSampleRate, EResampleMode resamplingMode, nap::utility::ErrorState& errorState)
		{
			// Source and target are the same or buffer empty
			if (math::equal<float>(sourceSampleRate, destSampleRate) || buffer.getSize() == 0)
				return true;	

			// Resample
			float ratio = destSampleRate/sourceSampleRate;
			auto maxResampledSize = nap::math::ceil(buffer.getSize() * ratio);
			MultiSampleBuffer resampled(buffer.getChannelCount(), maxResampledSize);

			for(auto i = 0; i < buffer.getChannelCount(); i++)
			{
				SRC_DATA data;
				data.data_in = buffer[i].data();
				data.data_out = resampled[i].data();
				data.input_frames = buffer[i].size();
				data.output_frames = resampled[i].size();
				data.end_of_input =1;
				data.src_ratio = ratio;

				int ret = src_simple (&data, static_cast<uint>(resamplingMode), 1) ;
				if(ret != 0)
				{
					 errorState.fail("samplerate conversion failed: %s", src_strerror(ret));
					 return false;
				}
				
				if(data.input_frames_used != buffer[i].size()){
					nap::Logger::debug("Sample rate conversion: whole input not processed. input size: %u, processed samples: %d", buffer[i].size(), data.input_frames_used);
				}

				if(data.output_frames_gen != resampled[i].size())
				{
					nap::Logger::debug("Sample rate conversion: output is smaller than allocated size. %u, %d", resampled[i].size(), data.output_frames_gen);
					if(data.output_frames_gen > 0 && data.output_frames_gen < resampled[i].size())
						resampled[i].resize(data.output_frames_gen);						
				}
			}

			// Is it OK to swap?
			std::swap(buffer, resampled);
			return true;
		}
	}
}
