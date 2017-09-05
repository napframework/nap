#pragma once

#include "audiobufferresource.h"

namespace nap {
    
    namespace audio {
        
        class NAPAPI AudioFileResource : public AudioBufferResource {
            RTTI_ENABLE(AudioBufferResource)
        public:
            AudioFileResource() = default;
            bool init(utility::ErrorState& errorState) override;
            
        public:
            std::string mAudioFilePath = "";
            bool mAllowFailure = true;
        };
        
    }
    
}
