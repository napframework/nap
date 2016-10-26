// External Includes
#include <nap/logger.h>
#include <ofFileUtils.h>

// Local Includes
#include "napofsoundcomponent.h"

namespace nap
{
	// Constructor
	OFPlaySoundComponent::OFPlaySoundComponent()
	{
		// Connect signals / slots
		mFile.connectToValue(mFileChanged);
		mStream.connectToValue(mStreamChanged);
	}


	// Destructor stops playback and unloads audio file
	OFPlaySoundComponent::~OFPlaySoundComponent()
	{
		if (mPlayer.isLoaded())
		{
			if (mPlayer.isPlaying())
				mPlayer.stop();
			mPlayer.unload();
		}		
	}

	
	// Loads the file resource
	void OFPlaySoundComponent::loadFile()
	{
		// If empty, return
		if (mFile.getValue().empty())
			return;

		// Make sure the file exists
		if (!ofFile(mFile.getValue()).exists())
		{
			Logger::warn(this->getName() + ": file does not exist: " + mFile.getValue());
			return;
		}

		// If loaded, clear
		if (mPlayer.isLoaded())
		{
			mPlayer.unload();
		}

		// Load file
		mPlayer.load(mFile.getValue(), mStream.getValue());
		if (!mPlayer.isLoaded())
		{
			Logger::warn(this->getName() + ": unable to load file: " + mFile.getValue());
			return;
		}
	}
}


RTTI_DEFINE(nap::OFPlaySoundComponent)
