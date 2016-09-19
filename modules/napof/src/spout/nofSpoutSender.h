#pragma once

// OF Includes
#include <ofTexture.h>

// Spout includes
#include <Spout.h>

// Std includes
#include <string>

// Namespace
using namespace std;

/**
@brief Wrapper around Spout hardware texture sharing

This object is lazily initialized based on the constructor used or the SetTexture method
The name can not be longer than 255 characters!
**/

class NSpoutSender
{
public:

	///@name The texture sharing hardware mode to use
	enum class EHardwareMode : int
	{
		DX9,
		DX11,
	};

	///@name Construction / Destruction
	NSpoutSender(const string& inName);			
	~NSpoutSender()									{ mSpoutSender->ReleaseSender(); delete mSpoutSender; mSpoutSender = nullptr; }

	///@name Accessors
	const SpoutSender&	GetSenderRef() const		{ return *mSpoutSender; }
	void				SetHardwareMode(EHardwareMode inMode) {mHardwareMode = inMode; }
	bool				IsInitialized() const		{ return mInitialized; }
	bool				HardwareAccelerated() const	{ return !mMemoryShare; }

	///@name Sending
	void				SendTexture(ofTexture& inTexture);
	void				SetName(const std::string& inName) { mName = inName; }

private:
	SpoutSender*		mSpoutSender;

	///@name Properties
	unsigned int		mTextureWidth;				//< Send texture height
	unsigned int		mTextureHeight;				//< Send texture width
	bool				mInitialized;				//< If the sender is initialized
	bool				mHasSender;					//< If the initialization failed
	EHardwareMode		mHardwareMode;				//< Currently selected hardware sharing mode
	bool				mMemoryShare;				//< If the hardware accelerated texture sharing mode is not available
	string				mName;						//< The sender name

	///@name Initialization
	void				Initialize();				//< Initializes the sender
};
