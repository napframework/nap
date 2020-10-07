#pragma once

// std library includes
#include <string>

// nap includes
#include <audio/utility/audiotypes.h>
#include <utility/errorstate.h>

namespace nap
{
	
	namespace audio
	{
		
		/**
		 * Utility to read an audio file from disk
		 * @param fileName: the absolute path to the file
		 * @param buffer: the buffer to read the file into
		 * @param sampleRate: this will contain the sample rate of the audio file after success
		 * @return: true on success
		 */
		bool NAPAPI readAudioFile(const std::string& fileName, MultiSampleBuffer& output, float& outSampleRate,
		                          nap::utility::ErrorState& errorState);
		
	}
	
}
