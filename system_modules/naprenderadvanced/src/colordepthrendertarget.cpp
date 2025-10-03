/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "colordepthrendertarget.h"
#include "renderservice.h"
#include "textureutils.h"
#include "renderadvancedutils.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ColorDepthRenderTarget, "Color and Depth texture target for render operations")
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("ColorTexture",			&nap::ColorDepthRenderTarget::mColorTexture,			nap::rtti::EPropertyMetaData::Required, "The storage color texture")
	RTTI_PROPERTY("DepthTexture",			&nap::ColorDepthRenderTarget::mDepthTexture,			nap::rtti::EPropertyMetaData::Required, "The storage depth texture")
	RTTI_PROPERTY("SampleShading",			&nap::ColorDepthRenderTarget::mSampleShading,			nap::rtti::EPropertyMetaData::Default,	"Reduces texture aliasing at higher computational cost, applies to both color and depth")
	RTTI_PROPERTY("Samples",				&nap::ColorDepthRenderTarget::mRequestedSamples,		nap::rtti::EPropertyMetaData::Default,	"Number of MSAA samples to use, applies to both color and depth")
	RTTI_PROPERTY("ClearColor",				&nap::ColorDepthRenderTarget::mClearColor,				nap::rtti::EPropertyMetaData::Default,	"Initial clear color value")
	RTTI_PROPERTY("ClearDepth",				&nap::ColorDepthRenderTarget::mClearDepth,				nap::rtti::EPropertyMetaData::Default,	"Initial clear depth value")
RTTI_END_CLASS

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // Static functions
    //////////////////////////////////////////////////////////////////////////

	// Creates the color image and view
	static bool createColorResource(const RenderService& renderer, VkExtent2D targetSize, VkFormat colorFormat, VkSampleCountFlagBits sampleCount, ImageData& outData, utility::ErrorState& errorState)
	{
		if (!create2DImage(renderer.getVulkanAllocator(), targetSize.width, targetSize.height, colorFormat, 1, sampleCount, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY, outData.mImage, outData.mAllocation, outData.mAllocationInfo, errorState))
			return false;

		if (!create2DImageView(renderer.getDevice(), outData.getImage(), colorFormat, 1, VK_IMAGE_ASPECT_COLOR_BIT, outData.mView, errorState))
			return false;

		return true;
	}


	// Create the depth image and view
	static bool createDepthResource(const RenderService& renderer, VkExtent2D targetSize, VkFormat depthFormat, VkSampleCountFlagBits sampleCount, VkImageUsageFlags usage, ImageData& outImage, utility::ErrorState& errorState)
	{
		if (!create2DImage(renderer.getVulkanAllocator(), targetSize.width, targetSize.height, depthFormat, 1, sampleCount, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | usage, VMA_MEMORY_USAGE_GPU_ONLY, outImage.mImage, outImage.mAllocation, outImage.mAllocationInfo, errorState))
			return false;

		if (!create2DImageView(renderer.getDevice(), outImage.getImage(), depthFormat, 1, VK_IMAGE_ASPECT_DEPTH_BIT, outImage.mView, errorState))
			return false;

		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	// ColorDepthRenderTarget
	//////////////////////////////////////////////////////////////////////////

	ColorDepthRenderTarget::ColorDepthRenderTarget(Core& core) :
		mRenderService(core.getService<RenderService>()),
		mTextureLink(*this)
	{}


	ColorDepthRenderTarget::~ColorDepthRenderTarget()
	{
		if (mFramebuffer != VK_NULL_HANDLE)
			vkDestroyFramebuffer(mRenderService->getDevice(), mFramebuffer, nullptr);
	
		if (mRenderPass != VK_NULL_HANDLE)
			vkDestroyRenderPass(mRenderService->getDevice(), mRenderPass, nullptr);

		utility::destroyImageAndView(mDepthImage, mRenderService->getDevice(), mRenderService->getVulkanAllocator());
		utility::destroyImageAndView(mColorImage, mRenderService->getDevice(), mRenderService->getVulkanAllocator());
	}


	bool ColorDepthRenderTarget::init(utility::ErrorState& errorState)
	{
		// Warn if requested number of samples is not matched by hardware
		if (!mRenderService->getRasterizationSamples(mRequestedSamples, mRasterizationSamples, errorState))
			nap::Logger::warn(errorState.toString().c_str());

		// Check if sample rate shading is enabled
		if (mSampleShading && !(mRenderService->sampleShadingSupported()))
		{
			nap::Logger::warn("Sample shading requested but not supported");
			mSampleShading = false;
		}

		// Set framebuffer size
		const auto size = mColorTexture->getSize();
		VkExtent2D framebuffer_size = { (uint32)size.x, (uint32)size.y };

		// Store as attachments
		std::array<VkImageView, 4> attachments { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE };

		// Create render pass based on number of multi samples
		// When there's only 1 there's no need for a resolve step
		if (!createConsumeRenderPass(mRenderService->getDevice(), mColorTexture->getFormat(), mDepthTexture->getFormat(), mRasterizationSamples, getFinalLayout(), mRenderPass, errorState))
			return false;

		if (mRasterizationSamples == VK_SAMPLE_COUNT_1_BIT)
		{
			// Bind textures as attachments
			attachments[0] = std::as_const(*mColorTexture).getHandle().getView();
			attachments[1] = std::as_const(*mDepthTexture).getHandle().getView();
		}
		else
		{
			// Create multi-sampled color attachment
			if (!createColorResource(*mRenderService, framebuffer_size, mColorTexture->getFormat(), mRasterizationSamples, mColorImage, errorState))
				return false;

			// Create multi-sampled depth attachment
			if (!createDepthResource(*mRenderService, framebuffer_size, mDepthTexture->getFormat(), mRasterizationSamples, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, mDepthImage, errorState))
				return false;

			// Bind textures as attachments
			attachments[0] = mColorImage.getView();
			attachments[1] = mDepthImage.getView();
			attachments[2] = std::as_const(*mColorTexture).getHandle().getView();
			attachments[3] = std::as_const(*mDepthTexture).getHandle().getView();
		}

		// Create framebuffer
		VkFramebufferCreateInfo framebuffer_info = {};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = mRenderPass;
		framebuffer_info.attachmentCount = mRasterizationSamples == VK_SAMPLE_COUNT_1_BIT ? 2 : 4;
		framebuffer_info.pAttachments = attachments.data();
		framebuffer_info.width = framebuffer_size.width;
		framebuffer_info.height = framebuffer_size.height;
		framebuffer_info.layers = 1;

		return errorState.check(vkCreateFramebuffer(mRenderService->getDevice(), &framebuffer_info, nullptr, &mFramebuffer) == VK_SUCCESS,
			"Failed to create framebuffer");
	}


	void ColorDepthRenderTarget::beginRendering()
	{
		const auto size = mColorTexture->getSize();
		std::array<VkClearValue, 4> clear_values = {};
		clear_values[0].color = { mClearColor[0], mClearColor[1], mClearColor[2], mClearColor[3] };
		clear_values[1].depthStencil = { mClearDepth, 0 };
		clear_values[2].color = { mClearColor[0], mClearColor[1], mClearColor[2], mClearColor[3] };
		clear_values[3].depthStencil = { mClearDepth, 0 };

		VkRenderPassBeginInfo renderpass_info = {};
		renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpass_info.renderPass = mRenderPass;
		renderpass_info.framebuffer = mFramebuffer;
		renderpass_info.renderArea.offset = { 0, 0 };
		renderpass_info.renderArea.extent = { static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y) };
		renderpass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
		renderpass_info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(mRenderService->getCurrentCommandBuffer(), &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);

		VkRect2D rect = {};
		rect.offset = { 0, 0 };
		rect.extent = { static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y) };
		vkCmdSetScissor(mRenderService->getCurrentCommandBuffer(), 0, 1, &rect);

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = size.y;
		viewport.width = size.x;
		viewport.height = -size.y;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(mRenderService->getCurrentCommandBuffer(), 0, 1, &viewport);
	}


	void ColorDepthRenderTarget::endRendering()
	{
		vkCmdEndRenderPass(mRenderService->getCurrentCommandBuffer());

		// Sync image data with render pass final layout
		mTextureLink.sync(*mColorTexture);
		mTextureLink.sync(*mDepthTexture);
	}


	const glm::ivec2 ColorDepthRenderTarget::getBufferSize() const
	{
		return mColorTexture->getSize();
	}


	RenderTexture2D& ColorDepthRenderTarget::getColorTexture()
	{
		return *mColorTexture;
	}


	DepthRenderTexture2D& ColorDepthRenderTarget::getDepthTexture()
	{
		assert(mDepthTexture != nullptr);
		return *mDepthTexture;
	}


	VkFormat ColorDepthRenderTarget::getColorFormat() const
	{
		return mColorTexture->getFormat();
	}


	VkFormat ColorDepthRenderTarget::getDepthFormat() const
	{
		return mRenderService->getDepthFormat();
	}


	VkSampleCountFlagBits ColorDepthRenderTarget::getSampleCount() const
	{
		return mRasterizationSamples;
	}


	bool ColorDepthRenderTarget::getSampleShadingEnabled() const
	{
		return mSampleShading;
	}

} // nap
