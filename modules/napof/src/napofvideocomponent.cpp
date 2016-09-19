#include "napofvideocomponent.h"
#include <nap/logger.h>
#include <Utils/nofUtils.h>

#include <ofxDSHapVideoPlayer.h>


namespace nap
{
	/**
	@brief Constructor
	**/
	OFVideoComponent::OFVideoComponent()
	{
		mASync.connectToValue(mSyncChanged);
		mFile.connectToValue(mFileChanged);

		// Set video player
		std::shared_ptr<ofxDSHapVideoPlayer> hap_player = std::make_shared<ofxDSHapVideoPlayer>();
		mPlayer.setPlayer(hap_player);

		mPaused.valueChangedSignal.connect(this, &OFVideoComponent::onPausedChanged);
	}


	/**
	@brief Destructor, closes video stream
	**/
	OFVideoComponent::~OFVideoComponent()
	{
		if (isLoaded())
		{
			mPlayer.close();
		}
	}


	/**
	@brief Loads the video file if it exists, call play to start playing

	Note that after this call the previous texture might be invalid
	**/
	void OFVideoComponent::loadVideo()
	{
		if (mFile.getValue().empty())
			return;

		mTexture.setValue(nullptr);

		// Ensure the new video file exists
		ofFile video_file(mFile.getValue());
		if (!video_file.exists())
		{
			Logger::fatal(this->getName() + ": Video file does not exist: " + video_file.getAbsolutePath());
			return;
		}

		// Close existing movie
		if (mPlayer.isLoaded())
		{
			if (mPlayer.isPlaying())
				mPlayer.stop();
			mPlayer.closeMovie();
		}

		// Load the video
		if (mASync.getValue())
		{
			mPlayer.loadAsync(video_file.getAbsolutePath());
		}
		else
		{
			mPlayer.load(video_file.getAbsolutePath());
		}

		// Warn on failure
		if (!mPlayer.isLoaded())
		{
			Logger::warn(this->getName() + ": unable to load video: " + video_file.getAbsolutePath());
		}

		// Init player
		mPlayer.setLoopState(mLoopMode);
        mPlayer.play();

		// Force update
		update();

		mTexture.setValue(&mPlayer.getTexture());
	}


	// Forces the player to display a certain frame
	void OFVideoComponent::setFrame(int inFrame)
	{
		if (!isLoaded())
		{
			nap::Logger::warn("No video loaded, can't set frame: %d", inFrame);
			return;
		}
        mHadHitLoopStart = false; // Reset so loopStartHit can be triggered again

		int frame = gClamp<int>(inFrame, 0, mPlayer.getTotalNumFrames() - 1);
		mPlayer.setFrame(frame);
		mPlayer.update();
	}

	/**
	@brief Updates the component
	**/
	void OFVideoComponent::onUpdate()
	{
        bool wasPlaying = mPlayer.isPlaying();

		mPlayer.update();

		// Emit signal when loop frame is hit
		if (mPlayer.getLoopState() == OF_LOOP_NONE && mPlayer.getCurrentFrame() >= mLoopFromFrame.getValue()) 
		{
			if (!mHadHitLoopStart) 
			{
				loopStartHit(*this);
				mHadHitLoopStart = true; // Make sure we're not continuously calling this after the loop point
			}
        }

		// If it's finished signal finish, if a loop frame was specified, loop from that frame onwards
		if (mPlayer.getIsMovieDone())
		{
			if (mLoopFromFrame > 0)
			{
				mPlayer.setFrame(mLoopFromFrame.getValue());
				mPlayer.play();
			}

            if (wasPlaying)
			    playbackFinished(*this);
		}
	}
}

RTTI_DEFINE(nap::OFVideoComponent)
