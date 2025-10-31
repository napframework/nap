/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "audiofileresource.h"

// audio includes
#include <audio/service/audioservice.h>

// nap includes
#include <nap/logger.h>

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
			float sampleRate;
			if (readAudioFile(mAudioFilePath, *getBuffer(), sampleRate, errorState))
			{
				if(mResample)
				{
					auto serviceSampleRate = mAudioService->getNodeManager().getSampleRate();
					if( serviceSampleRate != sampleRate)
					{
	
						nap::Logger::info("Resampling audio file: %s", mAudioFilePath.c_str());
						if(!resampleSampleBuffer(*getBuffer(), sampleRate, serviceSampleRate, static_cast<uint>(mResampleMode), errorState))
							return false;
						
						sampleRate = serviceSampleRate;
					}
				}
				nap::Logger::info("Loaded audio file: %s sampleRate: %f", mAudioFilePath.c_str(), sampleRate);
				setSampleRate(sampleRate);
				return true;
			}
			return false;
		}
		
		
		bool MultiAudioFileResource::init(utility::ErrorState& errorState)
		{
			float sampleRate = 0;
			
			if (mAudioFilePaths.empty())
			{
				errorState.fail("MultiAudioFileResource: need at least one audio file path");
				return false;
			}
			
			for (auto i = 0; i < mAudioFilePaths.size(); ++i)
			{
				if (readAudioFile(mAudioFilePaths[i], *getBuffer(), sampleRate, errorState))
				{

					if(mResample)
					{
						auto serviceSampleRate = mAudioService->getNodeManager().getSampleRate();
						if( serviceSampleRate != sampleRate)
						{
							nap::Logger::info("Resampling audio file: %s", mAudioFilePaths[i].c_str());
							if(!resampleSampleBuffer(*getBuffer(), sampleRate, serviceSampleRate, static_cast<uint>(mResampleMode), errorState))
								return false;
							
							sampleRate = serviceSampleRate;
						}
					}
				} else
					return false;
			}
			
			setSampleRate(sampleRate);
			
			
			return true;
		}
		
	}
}
