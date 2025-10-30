/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "audiobufferresource.h"

// Nap includes
#include <rtti/object.h>
#include <rtti/factory.h>
#include <nap/resourceptr.h>
#include <nap/core.h>

namespace nap
{
	namespace audio
	{
		
		// Forward declarations
		class AudioService;		
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
		 * Loads an audio file from disk into memory.
		 */
		class NAPAPI AudioFileResource : public AudioBufferResource
		{
			RTTI_ENABLE(AudioBufferResource)
		public:
			AudioFileResource(Core& core) : AudioBufferResource(core) { }
			
			// Inherited from AudioBufferResource
			bool init(utility::ErrorState& errorState) override;
		
		public:
			bool mResample = false; ///< property 'Resample' will resample the loaded audio file upon initing to match the samplerate of the selected sound device
			EResampleMode mResampleMode = EResampleMode::SincFastest; ///<property: 'ResampleMode' sets the resampling algorithm used.
			std::string mAudioFilePath = ""; ///< property: 'AudioFilePath' The path to the audio file on disk
		};
		
		
		/**
		 * An audio buffer resource whose content is made up out of the content of multiple audio files on disk.
		 * Each file adds it's channels as new channels to the resource. So two stereo files will make a quadro resource.
		 */
		class NAPAPI MultiAudioFileResource : public AudioBufferResource
		{
			RTTI_ENABLE(AudioBufferResource)
		
		public:
			MultiAudioFileResource(Core& core) : AudioBufferResource(core) { }
			
			// Inherited from AudioBufferResource
			bool init(utility::ErrorState& errorState) override;
		
		public:
			bool mResample = false; ///< property 'Resample' will resample the loaded audio file upon initing to match the samplerate of the selected sound device
			EResampleMode mResampleMode = EResampleMode::SincFastest; ///<property: 'ResampleMode' sets the resampling algorithm used.
			std::vector<std::string> mAudioFilePaths; ///< property: 'AudioFilePaths' The paths to the audio files on disk
		};

	}
}
