#include "audiofileresource.h"

// audio includes
#include <audio/service/audioservice.h>
#include <audio/utility/audiofilereader.h>

// RTTI
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioFileResource)
    RTTI_CONSTRUCTOR(nap::audio::AudioService&)
    RTTI_PROPERTY_FILELINK("AudioFilePath", &nap::audio::AudioFileResource::mAudioFilePath, nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Audio)
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
        
    }
    
}
