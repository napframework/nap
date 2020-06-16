#include "backbufferrendertarget.h"
#include "renderwindow.h"

namespace nap
{
	BackbufferRenderTarget::BackbufferRenderTarget(RenderWindow& window) :
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
