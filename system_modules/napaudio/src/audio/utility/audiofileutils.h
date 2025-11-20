/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// std library includes
#include <string>

// nap includes
#include <audio/utility/audiotypes.h>
#include <utility/errorstate.h>
#include <mathutils.h>

namespace nap
{
	namespace audio
	{
		/**
		 * Resampling algorithm options, taken from libsamplerate
		 * ordered by quality/speed. Better quality = longer processing times. 
		 * Best quality is very power hungry and takes about 10x more than the following algorithm
		*/
		enum class EResampleMode : uint
		{
			SincBestQuality			= 0,
			SincMediumQuality		= 1,
			SincFastest				= 2,
			ZeroOrderHold			= 3,
			Linear					= 4,
		};
		
		/**
		 * Utility to read an audio file from disk
		 * @param fileName the absolute path to the file
		 * @param output the buffer to read the file into
		 * @param outSampleRate  sample rate of the audio file
		 * @param errorState contains the error when reading fails
		 * @return true on success
		 */
		bool NAPAPI readAudioFile(const std::string& fileName, MultiSampleBuffer& output, float& outSampleRate, nap::utility::ErrorState& errorState);

		/**
		 * Utility to change the sample rate of a sample buffer. IMPORTANT: it is intended for resampling whole audio buffers from files not chuncks of audio.
		 * @param buffer the sample buffer that needs to be resampled. On success this buffer will contain the resampled audio data
		 * @param sourceSampleRate is the sample rate that is the sample buffer has.
		 * @param destSampleRate is the sample rate into which this function will resample the sample buffer.
		 * @param resamplingMode. use one of the enum values. These are based on libsamplerate modes. refer to https://libsndfile.github.io/libsamplerate/api_misc.html#converters
		 * @param errorState contains the error when resampling fails. If sourceSampleRate and destSampleRate are equal it will fail.
		 * @return true on success.
		 */
		bool NAPAPI resampleSampleBuffer(MultiSampleBuffer& buffer, float sourceSampleRate, float destSampleRate, EResampleMode resamplingMode, nap::utility::ErrorState& errorState);

		/**
		 * Creates a waveform of x segments by analyzing the RMS of the audio buffer.
		 * @param buffer the buffer to generate the RMS waveform from
		 * @param range the sample range of the audio buffer
		 * @param granularity number of samples to skip, 1 = don't skip; must be > 0
		 * @param bounds the computed amplitude bounds of the waveform, always between 0-1.
		 * @param ioBuffer the result of the RMS computation, where the number of samples = given size.
		 */
		void NAPAPI getWaveform(const SampleBuffer& buffer, const glm::ivec2& range, uint granularty, glm::vec2& bounds, SampleBuffer& ioBuffer);

		/**
		 * Creates a waveform of x segments by analyzing the RMS of the audio buffer.
		 * @param buffer the audio buffer to generate the waveform for
		 * @param granularity number of samples to skip, 1 = don't skip; must be > 0
		 * @param bounds the computed amplitude bounds of the waveform, always between 0-1.
		 * @param ioBuffer the result of the RMS computation, where the number of samples = given size.
		 */
		void NAPAPI getWaveform(const SampleBuffer& buffer, uint granularty, glm::vec2& bounds, SampleBuffer& ioBuffer);
	}
}
