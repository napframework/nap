/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "depthrendertarget.h"
#include "rendertexture2d.h"
#include "renderservice.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::DepthRenderTarget)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("DepthTexture",			&nap::DepthRenderTarget::mDepthTexture,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ClearValue",				&nap::DepthRenderTarget::mClearValue,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// RenderTarget
	//////////////////////////////////////////////////////////////////////////

	DepthRenderTarget::DepthRenderTarget(Core& core) :
		mRenderService(core.getService<RenderService>())
	{}


	DepthRenderTarget::~DepthRenderTarget()
	{
		if (mFramebuffer != VK_NULL_HANDLE)
			vkDestroyFramebuffer(mRenderService->getDevice(), mFramebuffer, nullptr);
	
		if (mRenderPass != VK_NULL_HANDLE)
			vkDestroyRenderPass(mRenderService->getDevice(), mRenderPass, nullptr);
	}


	bool DepthRenderTarget::init(utility::ErrorState& errorState)
	{
		// Set framebuffer size
		glm::uvec2 size = mDepthTexture->getSize();
		VkExtent2D framebuffer_size = { size.x, size.y };

		// Create depth render pass based on format
		if (!createDepthOnlyRenderPass(mRenderService->getDevice(), mDepthTexture->getFormat(), mRenderPass, errorState))
			return false;

		// Bind textures as attachments
		VkImageView attachment = mDepthTexture->getImageView();

		// Create framebuffer
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = mRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &attachment;
		framebufferInfo.width = framebuffer_size.width;
		framebufferInfo.height = framebuffer_size.height;
		framebufferInfo.layers = 1;

		if (!errorState.check(vkCreateFramebuffer(mRenderService->getDevice(), &framebufferInfo, nullptr, &mFramebuffer) == VK_SUCCESS, "Failed to create framebuffer"))
			return false;
		return true;
	}


	void DepthRenderTarget::beginRendering()
	{
		glm::ivec2 size = mDepthTexture->getSize();
		VkClearValue clear_value = {};
		clear_value.depthStencil = { mClearValue, 0 };

		// Setup render pass
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = mRenderPass;
		renderPassInfo.framebuffer = mFramebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = { static_cast<uint>(size.x), static_cast<uint>(size.y) };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clear_value;

		// Begin render pass
		vkCmdBeginRenderPass(mRenderService->getCurrentCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Ensure scissor and viewport are covering complete area
		VkRect2D rect;
		rect.offset.x = 0;
		rect.offset.y = 0;
		rect.extent.width = size.x;
		rect.extent.height = size.y;
		vkCmdSetScissor(mRenderService->getCurrentCommandBuffer(), 0, 1, &rect);

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = size.y;
		viewport.width = size.x;
		viewport.height = -size.y;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(mRenderService->getCurrentCommandBuffer(), 0, 1, &viewport);

		//// Depth bias (and slope) are used to avoid shadowing artifacts
		//// Constant depth bias factor (always applied)
		//float depthBiasConstant = 1.25f;

		//// Slope depth bias factor, applied depending on polygon's slope
		//float depthBiasSlope = 1.75f;

		//vkCmdSetDepthBias(mRenderService->getCurrentCommandBuffer(), depthBiasConstant, 0, depthBiasSlope);
	}


	void DepthRenderTarget::endRendering()
	{
		vkCmdEndRenderPass(mRenderService->getCurrentCommandBuffer());
	}


	const glm::ivec2 DepthRenderTarget::getBufferSize() const
	{
		return mDepthTexture->getSize();
	}


	DepthRenderTexture2D& DepthRenderTarget::getDepthTexture()
	{
		return *mDepthTexture;
	}


	VkFormat DepthRenderTarget::getColorFormat() const
	{
		return VK_FORMAT_UNDEFINED;
	}


	VkFormat DepthRenderTarget::getDepthFormat() const
	{
		return mDepthTexture->getFormat();
	}
} // nap
