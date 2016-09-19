#include <Spout/nofSpoutSender.h>
#include <assert.h>

NSpoutSender::NSpoutSender(const string& inName) : mTextureWidth(0), mTextureHeight(0), mName(inName),
	mInitialized(false), mHardwareMode(EHardwareMode::DX11), mMemoryShare(false), mHasSender(false)
{
	mSpoutSender = new SpoutSender();
}



/**
@brief Initializes the spout sender using the stored settings
**/
void NSpoutSender::Initialize()
{
	assert(!mName.empty());
	assert(mTextureHeight > 0 && mTextureWidth > 0);

	// Deinitialize before initializing again
	if (mInitialized)
	{
		mSpoutSender->ReleaseSender();
		mInitialized = false;
	}

	// Set the hardware acceleration mode
	bool dx_9 = mHardwareMode == EHardwareMode::DX9 ? true : false;
	mSpoutSender->SetDX9(dx_9);

	// Create sender name
	char sender_name[255];
	strcpy_s(sender_name, mName.c_str());

	// Initialize
	mHasSender = mSpoutSender->CreateSender(sender_name, mTextureWidth, mTextureHeight);
	
	// Make sure we're initialized now
	if (!mHasSender)
	{
		ofLogError("Spout") << "Unable to initialize Spout Sender";
		return;
	}

	// Toggle init
	mInitialized = true;

	// Get memory share mode
	mMemoryShare = mSpoutSender->GetMemoryShareMode();
}



/**
@brief Send a texture through Spout
**/
void NSpoutSender::SendTexture(ofTexture& inTexture)
{
	// Ensure the sender is initialized and the incoming texture has the right size
	if(!mInitialized || inTexture.getWidth() != mTextureWidth || inTexture.getHeight() != mTextureHeight)
	{
		mTextureWidth  = inTexture.getWidth();
		mTextureHeight = inTexture.getHeight();
		Initialize();
	}

	// Make sure we're initialized now
	if (!mHasSender)
		return;

	// Send the texture
	mSpoutSender->SendTexture(inTexture.getTextureData().textureID, GL_TEXTURE_2D, mTextureWidth, mTextureHeight, false);
}