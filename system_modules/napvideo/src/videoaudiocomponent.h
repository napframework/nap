/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "videoplayer.h"
#include "videoaudio.h"

// External Includes
#include <audio/utility/safeptr.h>
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
            rtti::ObjectPtr<VideoPlayer> mVideoPlayer = nullptr;	///< Property: 'VideoPlayer' The video player to output sound from.
            int mChannelCount = 2;									///< Property: 'ChannelCount' The number of channels of audio that will be requested from the Video object, defaults to 2.
			bool mProcessAudio = true;								///< Property: 'ProcessAudio' If the audio stream is decoded when available 
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
            OutputPin* getOutputForChannel(int channel) override	{ return &mNode->getOutput(channel); }
            
        private:
            SafeOwner<VideoNode> mNode = nullptr;					///< The audio node that polls the Video object for audio output
			VideoPlayer* mVideoPlayer = nullptr;					///< The video player device

			/**
			 * Called when the video changes.
			 * @param video the audio-video source.
			 */
			void selectVideo(VideoPlayer& video);

			// Called when video selection changes
			nap::Slot<VideoPlayer&> mVideoChangedSlot = { this, &VideoAudioComponentInstance::selectVideo };
        };
    }   
}
