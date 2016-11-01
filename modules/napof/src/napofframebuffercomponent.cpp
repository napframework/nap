#include "napofframebuffercomponent.h"
#include "utils/nofUtils.h"

namespace nap
{
	OFFrameBufferComponent::OFFrameBufferComponent()
	{
		// Connect
		Resolution.connectToValue(mResolutionChanged);
		initBuffer();
	}


	// Updates the frame buffer based on the provided settings
	void OFFrameBufferComponent::allocateBuffer()
	{
		mFrameBuffer.allocate(mSettings);
	}


	// Sets the new buffer settings and updates the fbo
	void OFFrameBufferComponent::setSettings(const ofFbo::Settings& inSettings)
	{
		// Store settings
		mSettings = inSettings;

		// Store resolution
		Resolution.getValueRef() = ofVec2i(inSettings.width, inSettings.height);

		// Allocate buffer
		allocateBuffer();
	}


	// Sets the new width and height
	void OFFrameBufferComponent::resolutionChanged(const ofVec2i& inValue)
	{
		mSettings.width  = inValue.x;
		mSettings.height = inValue.y;
		allocateBuffer();
	}


	// Initializes the frame buffer on construction
	void OFFrameBufferComponent::initBuffer()
	{
		// Populate buffer
		mSettings.width = Resolution.getValue().x;
		mSettings.height = Resolution.getValue().y;
		mSettings.wrapModeVertical   = GL_MIRRORED_REPEAT;
		mSettings.wrapModeHorizontal = GL_MIRRORED_REPEAT;
		mSettings.minFilter = GL_LINEAR;
		mSettings.maxFilter = GL_LINEAR;
		mSettings.internalformat = GL_RGBA;
		mSettings.useDepth = true;

		// Allocate
		allocateBuffer();
	}


	// Begin
	void OFFrameBufferComponent::begin()
	{
		// Reset color (white)
		ofSetColor(gDefaultWhiteColor);

		// Begin painting
		mFrameBuffer.begin();

		// Clear data in existing buffer
		ofClear(gDefaultTransparentBackColor);
	}
}

RTTI_DEFINE(nap::OFFrameBufferComponent)
