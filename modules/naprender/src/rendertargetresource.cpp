// Local Includes
#include "rendertargetresource.h"

namespace nap
{
	// Non const frame buffer getter
	opengl::FramebufferBase& RenderTargetResource::getTarget()
	{
		return const_cast<opengl::FramebufferBase&>(static_cast<const RenderTargetResource&>(*this).getTarget());
	}

	// Frame buffer constructor
	FrameBufferResource::FrameBufferResource()
	{
		size.valueChanged.connect(dimensionsChanged);
	}

	// Return the frame buffer, initialize if necessary
	const opengl::FramebufferBase& FrameBufferResource::getTarget() const
	{
		// If the framebuffer hasn't been loaded, do so
		if (!mLoaded)
		{
			mFrameBuffer.init();
			if (!mFrameBuffer.isValid())
			{
				nap::Logger::warn("unable to validate frame buffer: %s", getResourcePath().c_str());
			}
			mLoaded = true;
		}

		// Check if the size has changed, if so re-allocate textures for buffer
		if (mDirty)
		{
			mFrameBuffer.allocate(size.getValue().x, size.getValue().y);
			mDirty = false;
		}

		return mFrameBuffer;
	}


	// Resource path is display name for frame buffer
	std::string FrameBufferResource::getDisplayName() const
	{
		return getResourcePath();
	}

} // nap

RTTI_DEFINE(nap::RenderTargetResource)
RTTI_DEFINE(nap::FrameBufferResource)