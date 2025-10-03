/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "depthrendertarget.h"
#include "rendertexture2d.h"
#include "renderservice.h"
#include "renderadvancedutils.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::DepthRenderTarget, "Depth texture target for render operations")
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("DepthTexture",			&nap::DepthRenderTarget::mDepthTexture,			nap::rtti::EPropertyMetaData::Required, "The storage depth texture")
	RTTI_PROPERTY("SampleShading",			&nap::DepthRenderTarget::mSampleShading,		nap::rtti::EPropertyMetaData::Default,	"Reduces texture aliasing at higher computational cost")
	RTTI_PROPERTY("Samples",				&nap::DepthRenderTarget::mRequestedSamples,		nap::rtti::EPropertyMetaData::Default,	"Number of MSAA samples to use")
	RTTI_PROPERTY("ClearValue",				&nap::DepthRenderTarget::mClearValue,			nap::rtti::EPropertyMetaData::Default,	"Initial clear value")
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static functions
	//////////////////////////////////////////////////////////////////////////

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
	// DepthRenderTarget
	//////////////////////////////////////////////////////////////////////////

	DepthRenderTarget::DepthRenderTarget(Core& core) :
		mRenderService(core.getService<RenderService>()),
		mTextureTargetLink(*this)
	{}


	DepthRenderTarget::~DepthRenderTarget()
	{
		if (mFramebuffer != VK_NULL_HANDLE)
			vkDestroyFramebuffer(mRenderService->getDevice(), mFramebuffer, nullptr);
	
		if (mRenderPass != VK_NULL_HANDLE)
			vkDestroyRenderPass(mRenderService->getDevice(), mRenderPass, nullptr);

		utility::destroyImageAndView(mDepthImage, mRenderService->getDevice(), mRenderService->getVulkanAllocator());
	}


	bool DepthRenderTarget::init(utility::ErrorState& errorState)
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

		// Assign clear color
		mClearColor = { mClearValue, mClearValue, mClearValue, mClearValue };

		// Set framebuffer size
		glm::uvec2 size = mDepthTexture->getSize();
		VkExtent2D framebuffer_size = { size.x, size.y };

		// Create depth render pass based on format
		if (!createDepthOnlyRenderPass(mRenderService->getDevice(), mDepthTexture->getFormat(), mRasterizationSamples, getFinalLayout(), mRenderPass, errorState))
			return false;

		// Store as attachments
		std::array<VkImageView, 2> attachments { std::as_const(*mDepthTexture).getHandle().getView(), VK_NULL_HANDLE };
		if (mRasterizationSamples != VK_SAMPLE_COUNT_1_BIT)
		{
			// Create depth image data and hook up to depth attachment
			if (!createDepthResource(*mRenderService, framebuffer_size, mDepthTexture->getFormat(), mRasterizationSamples, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, mDepthImage, errorState))
				return false;

			// Reorder
			attachments[1] = attachments[0];
			attachments[0] = mDepthImage.getView();
		}

		// Create framebuffer
		VkFramebufferCreateInfo framebuffer_info = {};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = mRenderPass;
		framebuffer_info.attachmentCount = mRasterizationSamples != VK_SAMPLE_COUNT_1_BIT ? 2 : 1;
		framebuffer_info.pAttachments = attachments.data();
		framebuffer_info.width = framebuffer_size.width;
		framebuffer_info.height = framebuffer_size.height;
		framebuffer_info.layers = 1;

		if (!errorState.check(vkCreateFramebuffer(mRenderService->getDevice(), &framebuffer_info, nullptr, &mFramebuffer) == VK_SUCCESS, "Failed to create framebuffer"))
			return false;

		return true;
	}


	void DepthRenderTarget::beginRendering()
	{
		VkClearValue clear_value = {};
		clear_value.depthStencil = { mClearValue, 0 };

		glm::ivec2 size = mDepthTexture->getSize();
		const glm::ivec2 offset = { 0, 0 };

		// Setup render pass
		VkRenderPassBeginInfo renderpass_info = {};
		renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpass_info.renderPass = mRenderPass;
		renderpass_info.framebuffer = mFramebuffer;
		renderpass_info.renderArea.offset = { offset.x, offset.y };
		renderpass_info.renderArea.extent = { static_cast<uint>(size.x), static_cast<uint>(size.y) };
		renderpass_info.clearValueCount = 1;
		renderpass_info.pClearValues = &clear_value;

		// Begin render pass
		vkCmdBeginRenderPass(mRenderService->getCurrentCommandBuffer(), &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);

		// Ensure scissor and viewport are covering complete area
		VkRect2D rect;
		rect.offset.x = offset.x;
		rect.offset.y = offset.y;
		rect.extent.width = size.x;
		rect.extent.height = size.y;
		vkCmdSetScissor(mRenderService->getCurrentCommandBuffer(), 0, 1, &rect);

		VkViewport viewport = {};
		viewport.x = static_cast<float>(offset.x);
		viewport.y = size.y + static_cast<float>(offset.y);
		viewport.width = size.x;
		viewport.height = -size.y;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(mRenderService->getCurrentCommandBuffer(), 0, 1, &viewport);
	}


	void DepthRenderTarget::endRendering()
	{
		// End render pass and sync layout
		vkCmdEndRenderPass(mRenderService->getCurrentCommandBuffer());
		mTextureTargetLink.sync(*mDepthTexture);
	}


	const glm::ivec2 DepthRenderTarget::getBufferSize() const
	{
		return mDepthTexture->getSize();
	}


	DepthRenderTexture2D& DepthRenderTarget::getDepthTexture()
	{
		return *mDepthTexture;
	}


	VkFormat DepthRenderTarget::getDepthFormat() const
	{
		return mDepthTexture->getFormat();
	}
}
