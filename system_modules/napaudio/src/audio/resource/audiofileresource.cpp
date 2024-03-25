/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "audiofileresource.h"

// audio includes
#include <audio/service/audioservice.h>
#include <audio/utility/audiofileutils.h>

// nap includes
#include <nap/logger.h>

// RTTI
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioFileResource)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY_FILELINK("AudioFilePath", &nap::audio::AudioFileResource::mAudioFilePath, nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Audio)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::MultiAudioFileResource)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("AudioFilePaths", &nap::audio::MultiAudioFileResource::mAudioFilePaths, nap::rtti::EPropertyMetaData::Required)
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
				nap::Logger::info("Loaded audio file: %s", mAudioFilePath.c_str());
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
					nap::Logger::info("Loaded audio file: %s", mAudioFilePaths[i].c_str());
					if (i == 0)
						setSampleRate(sampleRate);
					else {
						if (sampleRate != getSampleRate()) {
							errorState.fail("MultiAudioFileResource: files have different sample rates.");
							return false;
						}
					}
				} else
					return false;
			}
			
			return true;
		}
		
	}
}
