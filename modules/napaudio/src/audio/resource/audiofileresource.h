#pragma once

#include "audiobufferresource.h"

// Nap includes
#include <rtti/object.h>
#include <rtti/factory.h>

namespace nap
{
    
    namespace audio
    {
        
        // Forward declarations
        class AudioService;
        
        /**
         * An audio file from disk loaded into memory.
         */
        class NAPAPI AudioFileResource : public AudioBufferResource
        {
            RTTI_ENABLE(AudioBufferResource)
        public:
            AudioFileResource(AudioService& service) : AudioBufferResource(service) { }
            bool init(utility::ErrorState& errorState) override;
            
        public:
            std::string mAudioFilePath = ""; ///< property: 'AudioFilePath' The path to the audio file on disk
        };
        
        using AudioFileResourceObjectCreator = rtti::ObjectCreator<AudioFileResource, AudioService>;
        
    }
    
}
