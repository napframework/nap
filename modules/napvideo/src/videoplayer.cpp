/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "videoplayer.h"

// Local Includes
#include "videoservice.h"

// External Includes
#include <mathutils.h>
#include <nap/assert.h>
#include <libavformat/avformat.h>

// nap::videoplayer run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VideoPlayer)
	RTTI_CONSTRUCTOR(nap::VideoService&)
	RTTI_PROPERTY("Loop",		&nap::VideoPlayer::mLoop,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("VideoFiles",	&nap::VideoPlayer::mVideoFiles,		nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("VideoIndex",	&nap::VideoPlayer::mVideoIndex,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Speed",		&nap::VideoPlayer::mSpeed,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	VideoPlayer::VideoPlayer(VideoService& service) :
		mService(service)
	{ }


	const nap::VideoFile& VideoPlayer::getFile() const
	{
		assert(mCurrentVideoIndex < mVideoFiles.size());
		return *mVideoFiles[mCurrentVideoIndex];
	}


	const nap::Video& VideoPlayer::getVideo() const
	{
		NAP_ASSERT_MSG(mCurrentVideo != nullptr, "No video selected");
		return *mCurrentVideo;
	}


	nap::Video& VideoPlayer::getVideo()
	{
		NAP_ASSERT_MSG(mCurrentVideo != nullptr, "No video selected");
		return *mCurrentVideo;
	}


	bool VideoPlayer::selectVideo(int index, utility::ErrorState& error)
	{
		// Update video index, bail if it's the same as we have currently selected
		NAP_ASSERT_MSG(!mVideos.empty(), "No video contexts available, call start() before video selection");
		int new_idx = math::clamp<int>(index, 0, mVideos.size() - 1);
		if (mVideos[new_idx].get() == mCurrentVideo)
			return true;

		// Stop playback of current video if available
		if (mCurrentVideo != nullptr)
			mCurrentVideo->stop(true);

		// Update selection
		mCurrentVideo = mVideos[new_idx].get();
		mCurrentVideoIndex = new_idx;

		// Store new width and height
		float vid_x = mCurrentVideo->getWidth();
		float vid_y = mCurrentVideo->getHeight();

		// Copy properties for playback
		mCurrentVideo->mLoop  = mLoop;
		mCurrentVideo->mSpeed = mSpeed;

		// Check if textures need to be generated, this is the case when there are none,
		// or when the dimensions have 
		if (!mTexturesCreated || vid_x != mYTexture->getWidth() || vid_y != mYTexture->getHeight())
		{
			// Create texture description
			SurfaceDescriptor tex_description;
			tex_description.mWidth = vid_x;
			tex_description.mHeight = vid_y;
			tex_description.mColorSpace = EColorSpace::Linear;
			tex_description.mDataType = ESurfaceDataType::BYTE;
			tex_description.mChannels = ESurfaceChannels::R;

			// Create Y Texture
			mYTexture = std::make_unique<Texture2D>(mService.getCore());
			mYTexture->mUsage = ETextureUsage::DynamicWrite;
			if (!mYTexture->init(tex_description, false, Texture2D::EClearMode::Clear, 0, error))
				return false;

			// Update dimensions for U and V texture
			float uv_x = vid_x * 0.5f;
			float uv_y = vid_y * 0.5f;
			tex_description.mWidth  = uv_x;
			tex_description.mHeight = uv_y;

			// Create U
			mUTexture = std::make_unique<Texture2D>(mService.getCore());
			mUTexture->mUsage = ETextureUsage::DynamicWrite;
			if (!mUTexture->init(tex_description, false, Texture2D::EClearMode::Clear, 0, error))
				return false;

			// Create V Texture
			mVTexture = std::make_unique<Texture2D>(mService.getCore());
			mVTexture->mUsage = ETextureUsage::DynamicWrite;
			if (!mVTexture->init(tex_description, false, Texture2D::EClearMode::Clear, 0, error))
				return false;

			mTexturesCreated = true;
		}

		// Notify listeners
		VideoChanged(*this);
		return true;
	}


	void VideoPlayer::clearTextures()
	{
		float vid_x  = getVideo().getWidth();
		float vid_y  = getVideo().getHeight();
		float uv_x = vid_x * 0.5f;
		float uv_y = vid_y * 0.5f;

		// YUV420p to RGB conversion uses an 'offset' value of (-0.0625, -0.5, -0.5) in the shader. 
		// This means that initializing the YUV planes to zero does not actually result in black output.
		// To fix this, we initialize the YUV planes to the negative of the offset
		std::vector<uint8_t> y_default_data(vid_x * vid_y, 16);

		// Initialize UV planes
		std::vector<uint8_t> uv_default_data(uv_x * uv_y, 127);

		mYTexture->update(y_default_data.data(), mYTexture->getWidth(), mYTexture->getHeight(), mYTexture->getWidth(), ESurfaceChannels::R);
		mUTexture->update(uv_default_data.data(), mUTexture->getWidth(), mUTexture->getHeight(), mUTexture->getWidth(), ESurfaceChannels::R);
		mVTexture->update(uv_default_data.data(), mVTexture->getWidth(), mVTexture->getHeight(), mVTexture->getWidth(), ESurfaceChannels::R);
	}


	bool VideoPlayer::start(utility::ErrorState& errorState)
	{
		// Ensure there's at least 1 video
		if (!errorState.check(mVideoFiles.size() > 0, "Playlist is empty"))
			return false;

		// Create all the unique video objects
		mVideos.clear();
		for (const auto& file : mVideoFiles)
		{
			// Create video and initialize
			std::unique_ptr<nap::Video> new_video = std::make_unique<nap::Video>(file->mPath);
			if (!new_video->init(errorState))
			{
				errorState.fail("%s: Unable to load video for file: %s", mID.c_str(), file->mPath.c_str());
				return false;
			}
			mVideos.emplace_back(std::move(new_video));
		}

		// Now select video, creates textures if required
		if (!selectVideo(mVideoIndex, errorState))
			return false;

		// Register device
		mService.registerVideoPlayer(*this);
		return true;
	}


	void VideoPlayer::play(double mStartTime)
	{
		// Clear textures and start playback
		clearTextures();
		getVideo().play(mStartTime);
	}


	void VideoPlayer::loop(bool value)
	{
		mLoop = value;
		getVideo().mLoop = mLoop;
	}


	void VideoPlayer::setSpeed(float speed)
	{
		mSpeed = speed;
		getVideo().mSpeed = speed;
	}


	void VideoPlayer::stop()
	{
		// Unregister player
		mService.removeVideoPlayer(*this);

		// Clear all videos
		mVideos.clear();
		mCurrentVideo = nullptr;
		mCurrentVideoIndex = 0;
	}


	nap::Texture2D& VideoPlayer::getYTexture()
	{
		NAP_ASSERT_MSG(mYTexture != nullptr, "Missing video Y texture");
		return *mYTexture;
	}


	nap::Texture2D& VideoPlayer::getUTexture()
	{
		NAP_ASSERT_MSG(mUTexture != nullptr, "Missing video U texture");
		return *mUTexture;
	}

	
	nap::Texture2D& VideoPlayer::getVTexture()
	{
		NAP_ASSERT_MSG(mVTexture != nullptr, "Missing video V texture");
		return *mVTexture;
	}


	void VideoPlayer::update(double deltaTime)
	{
		// Bail if there's no selection or playback is disabled
		if (mCurrentVideo == nullptr || !mCurrentVideo->isPlaying())
			return;

		// Get frame and update contents
		Frame new_frame = mCurrentVideo->update(deltaTime);
		if (new_frame.isValid())
		{
			// Copy data into texture
			assert(mYTexture != nullptr);
			mYTexture->update(new_frame.mFrame->data[0], mYTexture->getWidth(), mYTexture->getHeight(), new_frame.mFrame->linesize[0], ESurfaceChannels::R);
			mUTexture->update(new_frame.mFrame->data[1], mUTexture->getWidth(), mUTexture->getHeight(), new_frame.mFrame->linesize[1], ESurfaceChannels::R);
			mVTexture->update(new_frame.mFrame->data[2], mVTexture->getWidth(), mVTexture->getHeight(), new_frame.mFrame->linesize[2], ESurfaceChannels::R);
		}

		// Destroy frame that was allocated in the decode thread, after it has been processed
		new_frame.free();
	}
}