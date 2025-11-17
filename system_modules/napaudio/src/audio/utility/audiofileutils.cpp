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


		void getWaveform(const SampleBuffer& buffer, const glm::ivec2& range, uint granularty, glm::vec2& bounds, SampleBuffer& ioBuffer)
		{
			// Align range to granularity grid
			assert(range.x < range.y);
			assert(range.y < buffer.size());
			assert(range.x > -1);
			assert(granularty > 0);

			// Quantize
			size_t min = range.x;
			size_t max = range.y + 1;

			// Compute bucket size
			assert(!ioBuffer.empty());
			double bucket = (max - min) / static_cast<double>(ioBuffer.size());
			double thresh = math::min<double>(min + bucket, max);

			// Ensure step size doesn't exceed bucket size
			auto inc = math::min<uint>(bucket, granularty);

			// Initialize bounds
			bounds.x = math::max<float>();
			bounds.y = math::min<float>();

			size_t sct = 0;		//< Samples in bucket
			size_t pct = 0;		//< Previous bucket sample count
			size_t bct = 0;		//< Total number of buckets
			float rms = 0.0f;	//< Bucket amplitude

			for (auto i = min; i < max; i += inc)
			{
				// If current position overflows existing bucket, add it
				if (i >= thresh)
				{
					// Compute RMS for bucket 
					rms /= math::max<float>(sct, 1.0f);
					rms = sqrt(rms);
					pct = sct;

					// Update bounds
					bounds.x = rms < bounds.x ? rms : bounds.x;
					bounds.y = rms > bounds.y ? rms : bounds.y;

					// Average with previous sample, if available
					assert(bct < ioBuffer.size());
					ioBuffer[bct] = bct == 0 ? rms : (ioBuffer[bct - 1] + rms) / 2.0f;

					// Update bounds
					thresh += bucket;
					sct = 0; rms = 0.0f; bct++;
				}

				// Add sample for bucket
				rms += pow(buffer[i], 2.0f); sct++;
			}

			// Get RMS of remaining samples
			float sample_count = math::max<float>(sct, 1.0f);
			rms = sqrt(rms / sample_count);

			// Add previous bucket
			if (bct > 0)
			{
				float weight = pct / sample_count;
				rms += ioBuffer[bct - 1] * weight;
				rms /= weight + 1.0f;
			}

			ioBuffer.back() = rms;
		}


		void getWaveform(const SampleBuffer& buffer, uint granularity, glm::vec2& bounds, SampleBuffer& ioBuffer)
		{
			getWaveform(buffer, { 0, buffer.size() - 1 }, granularity, bounds, ioBuffer);
		}
	}
}
