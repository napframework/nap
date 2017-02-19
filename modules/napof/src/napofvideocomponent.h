#pragma once

// NAP Includes
#include <nap/attribute.h>
#include <nap/signalslot.h>

#include <napofattributes.h>
#include <napofupdatecomponent.h>

// OF Includes
#include <ofVideoPlayer.h>

namespace nap
{
	/**
	@brief Wraps an ofVideoplayer to be used in the NAP OF Framework
	**/
	class OFVideoComponent : public OFUpdatableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(OFUpdatableComponent)
	public:
		// Default constructor
		OFVideoComponent();
		virtual ~OFVideoComponent();


		// Slots
		NSLOT(mFileChanged, AttributeBase&, fileChanged)
		NSLOT(mSyncChanged, AttributeBase&, loadStateChanged)

		// Player
		const ofVideoPlayer& getPlayer() const	{ return mPlayer; }
		bool isInitialized() const				{ return mPlayer.isInitialized(); }
		bool isLoaded() const					{ return mPlayer.isLoaded(); }
		void play()								{ mPlayer.play(); }
		void stop()								{ 
			mHadHitLoopStart = false;
			mPlayer.stop();
		}
		void rewind()							{ 
			mHadHitLoopStart = false;
			mPlayer.setPosition(0);  
		}
		void setVolume(float inVolume)			{ mPlayer.setVolume(inVolume); }
		void mute()								{ mPlayer.setVolume(0.0f); }
		void pause(bool inValue)				{ mPlayer.setPaused(inValue); }
		bool isFrameNew() const					{ return mPlayer.isFrameNew(); }
		void setLoopMode(ofLoopType inType)		{ mLoopMode = inType;  mPlayer.setLoopState(mLoopMode); }
		ofLoopType getLoopType() const			{ return mLoopMode; }
		bool isPlaying()						{ return mPlayer.isPlaying(); }
		void setFrame(int inFrame);
		void skipToLastFrame()					{ setFrame(mPlayer.getTotalNumFrames() - 1); }
		

		// Utility
		float getWidth()						{ return mPlayer.getWidth(); }
		float getHeight()						{ return mPlayer.getHeight(); }

		void onPausedChanged(AttributeBase& attr) 
		{
			pause(attr.getValue<bool>());
		}

		// Override
		virtual void onUpdate() override;
 
		// Attributes
		Attribute<std::string>	mFile = { this, "VideoFile" };
		Attribute<bool>			mASync = { this, "ASync", false };
		Attribute<ofTexture*>   mTexture = { this, "videoTexture", nullptr };
		Attribute<bool>			mPaused = { this, "Paused", true};
        Attribute<int>			mLoopFromFrame = { this, "LoopFromFrame", 0 };

        Signal<OFVideoComponent&> playbackFinished;
        Signal<OFVideoComponent&> loopStartHit;

	private:
		ofVideoPlayer mPlayer;
		ofLoopType mLoopMode = OF_LOOP_NORMAL;
        bool mHadHitLoopStart = false;

		// Occurs when the file changes
		void fileChanged(AttributeBase& inName)		{ loadVideo(); }
		void loadStateChanged(AttributeBase& inState)		{ loadVideo(); }

		// Load the video
		void loadVideo();
	};
}

RTTI_DECLARE(nap::OFVideoComponent)
