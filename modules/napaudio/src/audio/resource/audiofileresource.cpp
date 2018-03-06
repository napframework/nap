#include "audiofileresource.h"

// audio includes
#include <audio/utility/audiofilereader.h>

// RTTI
RTTI_BEGIN_CLASS(nap::audio::AudioFileResource)
    RTTI_PROPERTY_FILELINK("AudioFilePath", &nap::audio::AudioFileResource::mAudioFilePath, nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Audio)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        bool AudioFileResource::init(utility::ErrorState& errorState)
        {
            float sampleRate;
            if (readAudioFile(mAudioFilePath, getBuffer(), sampleRate, errorState))
            {
                setSampleRate(sampleRate);
                return true;
            }
            return false;
        }
        
    }
    
}
