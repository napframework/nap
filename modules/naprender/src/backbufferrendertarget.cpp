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

	VkRenderPass BackbufferRenderTarget::getRenderPass() const
	{
		return mWindow.getRenderPass();
	}

	VkFormat BackbufferRenderTarget::getColorFormat() const
	{
		return mWindow.getSwapchainFormat();
	}

	VkFormat BackbufferRenderTarget::getDepthFormat() const
	{
		return mWindow.getDepthFormat();
	}


	VkSampleCountFlagBits BackbufferRenderTarget::getSampleCount() const
	{
		return mWindow.getSampleCount();
	}


	bool BackbufferRenderTarget::getSampleShadingEnabled() const
	{
		return mWindow.getSampleShadingEnabled();
	}

}
