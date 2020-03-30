#include "backbufferrendertarget.h"
#include "glwindow.h"

namespace nap
{
	BackbufferRenderTarget::BackbufferRenderTarget(GLWindow& window) :
		mWindow(window)
	{
	}

	void BackbufferRenderTarget::beginRendering()
	{
		mWindow.beginRenderPass();
	}

	void BackbufferRenderTarget::endRendering()
	{
		mWindow.endRenderPass();
	}
}
