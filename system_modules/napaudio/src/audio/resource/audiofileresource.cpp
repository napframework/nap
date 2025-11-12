/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "audiofileresource.h"

// audio includes
#include <audio/service/audioservice.h>

// nap includes
#include <nap/logger.h>
#include <mathutils.h>

// RTTI


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioFileResource, "Loads an audio file (.wav, .mp3, etc..) from disk into memory")
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY_FILELINK("AudioFilePath", &nap::audio::AudioFileResource::mAudioFilePath, nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Audio, "Path to the audio file (.wav, .mp3, etc..) on disk")
	RTTI_PROPERTY("Resample", &nap::audio::AudioFileResource::mResample,  nap::rtti::EPropertyMetaData::Default, "Will resample the loaded audio file upon initing to match the samplerate of the selected sound device")
	RTTI_PROPERTY("ResampleMode", &nap::audio::AudioFileResource::mResampleMode, nap::rtti::EPropertyMetaData::Default, "Resampling mode used when automatic resampling is enabled")
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::MultiAudioFileResource, "Loads multiple audio files from disk into memory as seperate channels, ie: 2x stereo '.wav' = 4 channels")
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("AudioFilePaths", &nap::audio::MultiAudioFileResource::mAudioFilePaths, nap::rtti::EPropertyMetaData::Required, "Paths to the audio files on disk to load")
	RTTI_PROPERTY("Resample", &nap::audio::MultiAudioFileResource::mResample,  nap::rtti::EPropertyMetaData::Default, "Will resample the loaded audio file upon initing to match the samplerate of the selected sound device")
	RTTI_PROPERTY("ResampleMode", &nap::audio::MultiAudioFileResource::mResampleMode, nap::rtti::EPropertyMetaData::Default, "Resampling mode used when automatic resampling is enabled")
RTTI_END_CLASS

namespace nap
{
	namespace audio
	{
		bool AudioFileResource::init(utility::ErrorState& errorState)
		{
			float samplerate;
			if (!readAudioFile(mAudioFilePath, *getBuffer(), samplerate, errorState))
				return false;

			auto service_samplerate = mAudioService->getNodeManager().getSampleRate();
			if(mResample &&  !math::equal<float>(service_samplerate, samplerate))
			{
				nap::Logger::debug("Resampling audio file: %s", mAudioFilePath.c_str());
				if(!resampleSampleBuffer(*getBuffer(), samplerate, service_samplerate, mResampleMode, errorState))
					return false;
				samplerate = service_samplerate;
				
			}
			nap::Logger::debug("Loaded audio file: %s samplerate: %.2f", mAudioFilePath.c_str(), samplerate);
			setSampleRate(samplerate);
			return true;
		}
		
		
		bool MultiAudioFileResource::init(utility::ErrorState& errorState)
		{
			
			if (mAudioFilePaths.empty())
			{
				errorState.fail("MultiAudioFileResource: need at least one audio file path");
				return false;
			}

			float samplerate = 0;
			auto service_samplerate = mAudioService->getNodeManager().getSampleRate();
			for (const auto& path : mAudioFilePaths)
			{
				if (!readAudioFile(path, *getBuffer(), samplerate, errorState))
					return false;

				if (mResample && !math::equal<float>(service_samplerate, samplerate))
				{
					nap::Logger::debug("Resampling audio file: %s", path.c_str());
					if (!resampleSampleBuffer(*getBuffer(), samplerate, service_samplerate, mResampleMode, errorState))
						return false;
					samplerate = service_samplerate;
				}
			}
			setSampleRate(samplerate);
			return true;
		}
		
	}
}
