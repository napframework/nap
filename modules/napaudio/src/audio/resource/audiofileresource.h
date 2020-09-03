#pragma once

#include "audiobufferresource.h"

// Nap includes
#include <rtti/object.h>
#include <rtti/factory.h>
#include <nap/resourceptr.h>

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

        
        /**
         * An audio buffer resource whose content is made up out of the content of multiple audio files on disk.
         * Each file adds it's channels as new channels to the resource. So two stereo files will make a quadro resource.
         */
        class NAPAPI MultiAudioFileResource : public AudioBufferResource
        {
            RTTI_ENABLE(AudioBufferResource)
        public:
//            class NAPAPI PathResource : public Resource
//            {
//                RTTI_ENABLE(Resource)
//            public:
//                std::string mAudioFilePath = "";
//            };
            
        public:
            MultiAudioFileResource(AudioService& service) : AudioBufferResource(service) { }
            bool init(utility::ErrorState& errorState) override;
            
        public:
            std::vector<std::string> mAudioFilePaths; ///< property: 'AudioFilePaths' The paths to the audio files on disk
        };
        

        using AudioFileResourceObjectCreator = rtti::ObjectCreator<AudioFileResource, AudioService>;
        using MultiAudioFileResourceObjectCreator = rtti::ObjectCreator<MultiAudioFileResource, AudioService>;

    }
    
}
