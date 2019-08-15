#pragma once

// Nap includes
#include <audio/utility/safeptr.h>

// Video includes
#include <video.h>
#include <videoaudio.h>

// Audio includes
#include <audio/component/audiocomponentbase.h>

namespace nap
{
    
    namespace audio
    {
		// Forward Declares
        class VideoAudioComponentInstance;
        
        /**
		 * Resource part of the VideoAudioComponent, outputs audio from a video stream.
		 * Without this component no video-audio is processed and therefore outputted.
         */
        class NAPAPI VideoAudioComponent : public AudioComponentBase
        {
            RTTI_ENABLE(AudioComponentBase)
            DECLARE_COMPONENT(VideoAudioComponent, VideoAudioComponentInstance)
            
        public:
            VideoAudioComponent() : AudioComponentBase() { }
            
            // Properties
            rtti::ObjectPtr<Video> mVideo = nullptr;	///< Property: 'Video' The Video that has audio.
            int mChannelCount = 2;						///< Property: 'ChannelCount' The number of channels of audio that will be requested from the Video object, defaults to 2.
        };

        
        /**
         * Instance part of the of VideoAudioComponent, outputs audio from a video stream.
		 * Without this component no video-audio is processed and therefore outputted.
         */
        class NAPAPI VideoAudioComponentInstance : public AudioComponentBaseInstance
        {
            RTTI_ENABLE(AudioComponentBaseInstance)
        public:
            VideoAudioComponentInstance(EntityInstance& entity, Component& resource) : AudioComponentBaseInstance(entity, resource) { }
            
            // Initialize the component
            bool init(utility::ErrorState& errorState) override;
            
            /**
             * @return the total number of audio channels
             */
            int getChannelCount() const override					{ return mNode->getChannelCount(); }

			/**
			 * @param channel index of the output channel.
			 * @return the output pin associated with the given channel.
			 */
            OutputPin& getOutputForChannel(int channel) override	{ return mNode->getOutput(channel); }

            /**
			 * @param video the audio-video source.
             */
            void setVideo(Video& video);
            
        private:
            SafeOwner<VideoNode> mNode = nullptr;					///< The audio node that polls the Video object for audio output
        };
    }   
}
