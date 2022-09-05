/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "audiobufferresource.h"

// Nap includes
#include <rtti/object.h>
#include <rtti/factory.h>
#include <nap/resourceptr.h>

namespace nap
{
	namespace audio
	{
		
		// Forward declarations
		class AudioService;
		
		/**
		 * Loads an audio file from disk into memory.
		 */
		class NAPAPI AudioFileResource : public AudioBufferResource
		{
			RTTI_ENABLE(AudioBufferResource)
		public:
			AudioFileResource(AudioService& service) : AudioBufferResource(service) { }
			
			// Inherited from AudioBufferResource
			bool init(utility::ErrorState& errorState) override;
		
		public:
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
			MultiAudioFileResource(AudioService& service) : AudioBufferResource(service) { }
			
			// Inherited from AudioBufferResource
			bool init(utility::ErrorState& errorState) override;
		
		public:
			std::vector<std::string> mAudioFilePaths; ///< property: 'AudioFilePaths' The paths to the audio files on disk
		};
		
		
		using AudioFileResourceObjectCreator = rtti::ObjectCreator<AudioFileResource, AudioService>;
		using MultiAudioFileResourceObjectCreator = rtti::ObjectCreator<MultiAudioFileResource, AudioService>;
		
	}
}
