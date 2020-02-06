#include "audiofileresource.h"

// audio includes
#include <audio/service/audioservice.h>
#include <audio/utility/audiofilereader.h>

// RTTI
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioFileResource)
    RTTI_CONSTRUCTOR(nap::audio::AudioService&)
    RTTI_PROPERTY_FILELINK("AudioFilePath", &nap::audio::AudioFileResource::mAudioFilePath, nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Audio)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::audio::MultiAudioFileResource::PathResource)
    RTTI_PROPERTY_FILELINK("AudioFilePath", &nap::audio::MultiAudioFileResource::PathResource::mAudioFilePath, nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Audio)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::MultiAudioFileResource)
    RTTI_CONSTRUCTOR(nap::audio::AudioService&)
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
                    if (i == 0)
                        setSampleRate(sampleRate);
                    else {
                        if (sampleRate != getSampleRate())
                        {
                            errorState.fail("MultiAudioFileResource: files have different sample rates.");
                            return false;
                        }
                    }
                }
                else
                    return false;
            }
            
            return true;
        }
        

    }
    
}
