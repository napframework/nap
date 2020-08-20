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
	 * videoplayer
	 */
	class NAPAPI VideoPlayer : public Device
	{
		RTTI_ENABLE(Device)
		friend class VideoService;
	public:

		/**
		 * Constructor
		 */
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
		 * Selects a new video for playback
		 */
		bool selectVideo(int index, utility::ErrorState& error);

		/**
	 	 * Starts playback of the current video at the given offset in seconds.
		 * @param startTimeSecs The offset in seconds to start the video at.
		 */
		void play(double startTime = 0.0);

		/**
		 * Stops playback of the current video without destroying the video context.
		 * @param blocking when set to true the thread calling this function will wait until playback is stopped.
		 */
		void stopPlayback(bool blocking)							{ getVideo().stop(blocking); }

		/**
		 * Check if the currently loaded video is playing
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
		 * Changes the playback speed of the player
		 */
		void setSpeed(float speed);

		/**
		 * @return current video playback speed
		 */
		float getSpeed() const										{ return mSpeed; }

		/**
		 * Seeks within the video to the time provided. This can be called while playing.
		 * @param seconds: the time offset in seconds in the videp.
		 */
		void seek(double seconds)									{ getVideo().seek(seconds); }

		/**
		 * @return The current playback position, in seconds.
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
		 * Starts the device
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
		 * @return The Y texture as it is updated by update(). Initially, the texture is not initialized
		 * to zero, but to the 'black' equivalent in YUV space. The size of the Y texture is width * height.
		 */
		RenderTexture2D& getYTexture();

		/**
		 * @return The U texture as it is updated by update(). Initially, the texture is not initialized
		 * to zero, but to the 'black' equivalent in YUV space. The size of the Y texture is HALF the width * height.
		 */
		RenderTexture2D& getUTexture();

		/**
		 * @return The V texture as it is updated by update(). Initially, the texture is not initialized
		 * to zero, but to the 'black' equivalent in YUV space. The size of the V texture is HALF the width * height.
		 */
		RenderTexture2D& getVTexture();

		std::vector<nap::ResourcePtr<VideoFile>> mVideoFiles;	///< Property: 'Files' All video file links
		nap::uint mVideoIndex = 0;								///< Property: 'Index' Selected video index
		bool mLoop = false;										///< Property: 'Loop' if the selected video loops
		float mSpeed = 1.0f;									///< Property: 'Speed' video playback speed
		nap::Signal<nap::VideoPlayer&> VideoChanged;			///< Called when the video selection changes

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
