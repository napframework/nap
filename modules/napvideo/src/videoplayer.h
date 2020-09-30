/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "videofile.h"
#include "video.h"

// External Includes
#include <nap/device.h>
#include <nap/resourceptr.h>
#include <nap/numeric.h>
#include <rendertexture2d.h>

namespace nap
{
	// Forward Declares
	class VideoService;

	/**
	 * Video playback device. 
	 *
	 * Decodes a video, from a list of videos, in a background thread and stores the result in a set of YUV textures.
	 * The textures are filled with the latest frame data on update(), which is a non-blocking operation.
	 * The nap::VideoService calls update() automatically each frame. 
	 *
	 * The main thread will consume the frames when they are present in the frame queue and their timestamp has 'passed'.
	 * Internally the textures are only updated when needed. YUV to RGB Conversion can be done in a shader.
	 *
	 * Videos are cached internally. This means that all videos in the playlist are created on start(), including the associated video format and video codec contexts.
	 * This ensures that a video can be started and stopped fast, without having to re-open a video when switching.
	 * The IO and decode threads are spawned on play() and stopped on stopPlayback().
	 *
	 * Note that the YUV textures are (re)-created when the video dimensions change from selection to selection.
	 * This is done to ensure there is always only 1 set of YUV textures allocated on the GPU per video player instance, 
	 * instead of a set of textures per video. A valid set of textures is always available after a successful call to selectVideo()
	 * Listen to the VideoChanged signal to get notified about a video change.
	 *
	 * Every video must contain a video stream, the audio stream is optional. 
	 * Use a nap::VideoAudioComponent to decode and play back the audio of a video.
	 * Without a nap::VideoAudioComponent no audio is decoded and therefore played back.
	 *
	 * Do not call start() and stop() at runtime, unless you want to completely restart the device and free all memory, 
	 * start() and stop() are called automatically by the engine at the appropiate time.
	 * Use play() and stopPlayback() instead to start and stop playback of the currently selected video.
	 * You always have to call play() after selecting a different video:
	 *
	 * ~~~~~{.cpp}
	 *	utility::ErrorState error;
	 *	if (!mVideoPlayer->selectVideo(new_selection, error))
	 *		nap::Logger::error(error.toString());
	 *	else
	 *		mVideoPlayer->play();
	 * ~~~~~
	 */
	class NAPAPI VideoPlayer : public Device
	{
		RTTI_ENABLE(Device)
		friend class VideoService;
	public:

		// Constructor
		VideoPlayer(VideoService& service);

		/**
		 * @return currently selected video file.
		 */
		const VideoFile& getFile() const;

		/**
		 * @return selected video context
		 */
		const Video& getVideo() const;								

		/**
		 * @return selected video context
		 */
		Video& getVideo();

		/**
		 * @return selected video index
		 */
		int getIndex() const										{ return mCurrentVideoIndex; }

		/**
		 * @return total number of available videos to choose from
		 */
		int getCount() const										{ return static_cast<int>(mVideos.size()); }

		/**
		 * Loads a new video to play, returns false if selection fails.
		 * The 'VideoChanged' signal is emitted on success.
		 * Call play() afterwards to start playback of the video.
		 *
		 * ~~~~~{.cpp}
		 *	utility::ErrorState error;
		 *	if (!mVideoPlayer->selectVideo(new_selection, error))
		 *		nap::Logger::error(error.toString());
		 *	else
		 *		mVideoPlayer->play();
		 * ~~~~~
		 *
		 * A new set of YUV textures is generated IF the new video has different dimensions or is the first to be selected.
		 * The old set of textures will be destroyed immediately and will be invalid after this call.
		 *
		 * @param index index of the video to load.
		 * @param error contains the error if changing the video fails.
		 * @return if video selection succeeded or failed
		 */
		bool selectVideo(int index, utility::ErrorState& error);

		/**
	 	 * Starts playback of the current video at the given offset in seconds.
		 * @param startTimeSecs The offset in seconds to start the video at.
		 */
		void play(double startTime = 0.0);

		/**
		 * Stops playback of the current video.
		 * @param blocking when set to true the thread calling this function will wait until playback is stopped.
		 */
		void stopPlayback()											{ getVideo().stop(true); }

		/**
		 * Check if the currently loaded video is playing.
		 * @return If the video is currently playing.
		 */
		bool isPlaying() const										{ return getVideo().isPlaying(); }

		/**
		 * If the video re-starts after completion.
		 * @param value if the video re-starts after completion.
		 */
		void loop(bool value);

		/**
		 * @return if the current video is looping
		 */
		bool isLooping() const										{ return mLoop; }

		/**
		 * Changes the playback speed of the player.
		 * @param speed new playback speed, 1.0f = default speed. 
		 */
		void setSpeed(float speed);

		/**
		 * @return current video playback speed
		 */
		float getSpeed() const										{ return mSpeed; }

		/**
		 * Seeks within the video to the time provided. This can be called while playing.
		 * @param seconds: the time offset in seconds in the video.
		 */
		void seek(double seconds)									{ getVideo().seek(seconds); }

		/**
		 * @return The current playback position in seconds.
		 */
		double getCurrentTime() const								{ return getVideo().getCurrentTime(); }

		/**
		 * @return The duration of the video in seconds.
		 */
		double getDuration() const									{ return getVideo().getDuration(); }

		/**
		 * @return Width of the video, in pixels.
		 */
		int getWidth() const										{ return getVideo().getWidth(); }

		/**
		 * @return Height of the video, in pixels.
		 */
		int getHeight() const										{ return getVideo().getHeight(); }

		/**
		 * @return Whether this video has an audio stream.
		 */
		bool hasAudio() const										{ return getVideo().hasAudio(); }

		/**
		 * Starts the device.
		 * @param errorState contains the error if the device can't be started
		 * @return if the device started
		 */
		virtual bool start(utility::ErrorState& errorState) override;

		/**
		 * Stops the device
		 */
		virtual void stop() override;

		/**
		 * @return if there is a video selected.
		 */
		bool hasSelection() const								{ return mCurrentVideo != nullptr; }

		/**
		 * Returns the decoded video Y texture.
		 * The texture is not initialized to zero, but to the 'black' equivalent in YUV space. 
		 * The size of the Y texture is width * height.
		 * @return The video Y texture.
		 */
		RenderTexture2D& getYTexture();

		/**
		 * Returns the decoded video U texture.
		 * The texture is not initialized to zero, but to the 'black' equivalent in YUV space.
		 * The size of the U texture is HALF the width * height.
		 * @return The video Y texture.
		 */
		RenderTexture2D& getUTexture();

		/**
		 * Returns the decoded video V texture.
		 * The texture is not initialized to zero, but to the 'black' equivalent in YUV space.
		 * The size of the V texture is HALF the width * height.
		 * @return The video V texture.
		 */
		RenderTexture2D& getVTexture();

		std::vector<nap::ResourcePtr<VideoFile>> mVideoFiles;	///< Property: 'Files' All video file links
		nap::uint mVideoIndex = 0;								///< Property: 'Index' Selected video index
		bool mLoop = false;										///< Property: 'Loop' if the selected video loops
		float mSpeed = 1.0f;									///< Property: 'Speed' video playback speed
		
		/**
		 * Emitted after a successful video switch.
		 */
		nap::Signal<nap::VideoPlayer&> VideoChanged;

	private:
		/**
		 * Update textures, can only be called by the video service
		 */
		void update(double deltaTime);

		/**
		 * Clear output textures to black
		 */
		void clearTextures();

		int	mCurrentVideoIndex = 0;								///< Current selected video index.		
		nap::Video* mCurrentVideo = nullptr;					///< Current selected video context
		bool mTexturesCreated = false;							///< If the textures have been created
		std::vector<std::unique_ptr<nap::Video>> mVideos;		///< All the actual videos
		std::unique_ptr<RenderTexture2D> mYTexture;				///< Video YTexture
		std::unique_ptr<RenderTexture2D> mUTexture;				///< Video UTexture
		std::unique_ptr<RenderTexture2D> mVTexture;				///< Video VTexture	
		VideoService&	mService;								///< Video service that this object is registered with
	};

	// Object creator used for constructing the the OSC receiver
	using VideoPlayerObjectCreator = rtti::ObjectCreator<VideoPlayer, VideoService>;
}
