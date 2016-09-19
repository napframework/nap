#pragma once

// NAP Includes
#include <nap/component.h>
#include <nap/attribute.h>
#include <nap/coremodule.h>
#include <napofattributes.h>

// OF Includes
#include <ofSoundPlayer.h>

namespace nap
{
	/**
	@brief NAP OpenFrameworks sound player

	Simple sound interface for sound file play back
	**/
	class OFPlaySoundComponent : public Component
	{
		RTTI_ENABLE_DERIVED_FROM(Component)

	public:
		// Default constructor
		OFPlaySoundComponent();
		virtual ~OFPlaySoundComponent();

		// Attributes
		Attribute<std::string> mFile =		{ this, "File" };
		Attribute<bool>	mStream =			{ this, "Stream", false };

		// Sound player
		const ofSoundPlayer& getPlayer()	{ return mPlayer; }

		// Interface
		bool isLoaded() const				{ return mPlayer.isLoaded(); }
		bool isPlaying() const				{ return mPlayer.isPlaying(); }
		void play()							{ mPlayer.setPaused(false); mPlayer.play(); }
		void stop()							{ mPlayer.stop(); }
		void pause(bool inValue)			{ mPlayer.setPaused(inValue); }
		void setLooping(bool inValue)		{ mPlayer.setLoop(inValue); }
		void setVolume(float inValue)		{ mPlayer.setVolume(inValue); }

		// Slots
		NSLOT(mFileChanged, const std::string&, fileChanged)
		NSLOT(mStreamChanged, const bool&, streamChanged)

	private:
		ofSoundPlayer	mPlayer;

		// Callbacks
		void fileChanged(const std::string& inFile)	{ loadFile(); }
		void streamChanged(const bool& inValue)		{ loadFile(); }

		// Load
		void loadFile();
	};
}

RTTI_DECLARE(nap::OFPlaySoundComponent)

