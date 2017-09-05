#include "audiofileresource.h"

// audio includes
#include "utility/audiofilereader.h"

// RTTI
RTTI_BEGIN_CLASS(nap::audio::AudioFileResource)
    RTTI_PROPERTY("AudioFilePath", &nap::audio::AudioFileResource::mAudioFilePath, nap::rtti::EPropertyMetaData::FileLink)
    RTTI_PROPERTY("AllowFailure", &nap::audio::AudioFileResource::mAllowFailure, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap {
    
    namespace audio {
        
        bool AudioFileResource::init(utility::ErrorState& errorState)
        {
            if (!readAudioFile(mAudioFilePath, mBuffer, mSampleRate))
                return errorState.check(mAllowFailure, "Failed to read audio file: " + mAudioFilePath);
            return true;
        }
        
    }
    
}
