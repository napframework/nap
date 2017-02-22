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
		if (!mLoaded)
		{
			mFrameBuffer.init();
			if (!mFrameBuffer.isValid())
			{
				nap::Logger::warn("unable to validate frame buffer: %s", getResourcePath().c_str());
			}
			else
			{
				mFrameBuffer.allocate(size.getValue().x, size.getValue().y);
			}
			mLoaded = true;
		}
		return mFrameBuffer;
	}


	// Resource path is display name for frame buffer
	std::string FrameBufferResource::getDisplayName() const
	{
		return getResourcePath();
	}


	// Allocated framebuffer based on new settings
	void FrameBufferResource::update()
	{
		opengl::FramebufferBase& target = RenderTargetResource::getTarget();
		static_cast<opengl::FrameBuffer&>(target).allocate(size.getValue().x, size.getValue().y);
	}

} // nap

RTTI_DEFINE(nap::RenderTargetResource)
RTTI_DEFINE(nap::FrameBufferResource)