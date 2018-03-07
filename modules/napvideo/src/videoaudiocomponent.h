#pragma once

// Video includes
#include <video.h>
#include <videoaudio.h>

// Audio includes
#include <audio/component/audiocomponentbase.h>

namespace nap
{
    
    namespace audio
    {
    
        class VideoAudioComponentInstance;
        
        /**
         * Audio object to output audio channels from a playing @Video object
         */
        class NAPAPI VideoAudioComponent : public AudioComponentBase
        {
            RTTI_ENABLE(AudioComponentBase)
            DECLARE_COMPONENT(VideoAudioComponent, VideoAudioComponentInstance)
            
        public:
            VideoAudioComponent() : AudioComponentBase() { }
            
            // Properties
            rtti::ObjectPtr<Video> mVideo = nullptr; ///< Property: 'Video' The @Video object that the audio channels are taken from
            int mChannelCount = 2; ///< Property: 'ChannelCount' The number of channels of audio that will be requested from the Video object
        };

        
        /**
         * Instance of an Audio object that outputs audio channels from a playing @Video object
         */
        class NAPAPI VideoAudioComponentInstance : public AudioComponentBaseInstance
        {
            RTTI_ENABLE(AudioComponentBaseInstance)
        public:
            VideoAudioComponentInstance(EntityInstance& entity, Component& resource) : AudioComponentBaseInstance(entity, resource) { }
            
            // Initialize the component
            bool init(utility::ErrorState& errorState) override;
            
            // Inherited from AudioComponentBaseInstance
            int getChannelCount() const override { return mNode->getChannelCount(); }
            OutputPin& getOutputForChannel(int channel) override { return mNode->getOutput(channel); }

            /**
             * Set the @Video object whose audio channels will be output
             */
            void setVideo(Video& video);
            
        private:
            std::unique_ptr<VideoNode> mNode = nullptr; ///< The audio node that polls the Video object for audio output
        };
        
    }
    
}
