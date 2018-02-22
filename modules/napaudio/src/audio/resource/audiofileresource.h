#pragma once

#include "audiobufferresource.h"

namespace nap
{
    
    namespace audio
    {
        
        /**
         * An audio file from disk loaded into memory.
         */
        class NAPAPI AudioFileResource : public AudioBufferResource
        {
            RTTI_ENABLE(AudioBufferResource)
        public:
            AudioFileResource() = default;
            bool init(utility::ErrorState& errorState) override;
            
        public:
            std::string mAudioFilePath = ""; ///< property: 'AudioFilePath' The path to the audio file on disk
        };
        
    }
    
}
