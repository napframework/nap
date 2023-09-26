/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "snapshotrendertarget.h"
#include "renderservice.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>

namespace nap
{
	namespace 
	{
		//////////////////////////////////////////////////////////////////////////
		// Static functions
		//////////////////////////////////////////////////////////////////////////
		
		// Creates a single or multi-sample renderpass based on rasterization samples and color/depth formats.
		// The renderpass is given an execution and memory dependency for resolving WAW hazards with color and depth attachments.
		static bool createSnapshotRenderPass(VkDevice device, VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits samples, VkRenderPass& renderPass, utility::ErrorState& errorState)
		{
			bool multi_sample = samples != VK_SAMPLE_COUNT_1_BIT;

			VkAttachmentDescription color_attachment = {};
			color_attachment.format = colorFormat;
			color_attachment.samples = samples;
			color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			color_attachment.finalLayout = !multi_sample ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentDescription depth_attachment = {};
			depth_attachment.format = depthFormat;
			depth_attachment.samples = samples;
			depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference color_attachment_ref = {};
			color_attachment_ref.attachment = 0;
			color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depth_attachment_ref = {};
			depth_attachment_ref.attachment = 1;
			depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &color_attachment_ref;
			subpass.pDepthStencilAttachment = &depth_attachment_ref;

			// WAW (write-after-write) hazard
			// This render pass does not read output from the previous render pass, but a memory dependency is still required to sync writes
			VkSubpassDependency dependency;
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;

			// The set of pipeline stages responsible for producing and consuming the color/depth attachments
			dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

			// The types of memory access that are involved with the src and dst stage masks
			dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			dependency.dependencyFlags = 0;

			VkRenderPassCreateInfo renderpass_info = {};
			renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderpass_info.subpassCount = 1;
			renderpass_info.pSubpasses = &subpass;
			renderpass_info.dependencyCount = 1;
			renderpass_info.pDependencies = &dependency;

			// Single-sample render pass
			if (!multi_sample)
			{
				std::array<VkAttachmentDescription, 2> attachments = { color_attachment, depth_attachment };
				renderpass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
				renderpass_info.pAttachments = attachments.data();
			}
			// Multi-sample render pass
			else
			{
				VkAttachmentDescription resolve_attachment = {};
				resolve_attachment.format = colorFormat;
				resolve_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
				resolve_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				resolve_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				resolve_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				resolve_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				resolve_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				resolve_attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				VkAttachmentReference resolve_attachment_ref {};
				resolve_attachment_ref.attachment = 2;
				resolve_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				subpass.pResolveAttachments = &resolve_attachment_ref;

				std::array<VkAttachmentDescription, 3> attachments = { color_attachment, depth_attachment, resolve_attachment };
				renderpass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
				renderpass_info.pAttachments = attachments.data();

				return errorState.check(vkCreateRenderPass(device, &renderpass_info, nullptr, &renderPass) == VK_SUCCESS, "Failed to create multi-sample render pass");
			}

			return errorState.check(vkCreateRenderPass(device, &renderpass_info, nullptr, &renderPass) == VK_SUCCESS, "Failed to create render pass");
		}
	}


	// Creates the color image and view
	static bool createColorResource(const RenderService& renderer, VkExtent2D targetSize, VkFormat colorFormat, VkSampleCountFlagBits sampleCount, ImageData& outData, utility::ErrorState& errorState)
	{
		if (!create2DImage(renderer.getVulkanAllocator(), targetSize.width, targetSize.height, colorFormat, 1, sampleCount, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY, outData.mImage, outData.mAllocation, outData.mAllocationInfo, errorState))
			return false;

		if (!create2DImageView(renderer.getDevice(), outData.getImage(), colorFormat, 1, VK_IMAGE_ASPECT_COLOR_BIT, outData.getView(), errorState))
			return false;

		return true;
	}


	// Create the depth image and view
	static bool createDepthResource(const RenderService& renderer, VkExtent2D targetSize, VkSampleCountFlagBits sampleCount, ImageData& outImage, utility::ErrorState& errorState)
	{
		if (!create2DImage(renderer.getVulkanAllocator(), targetSize.width, targetSize.height, renderer.getDepthFormat(), 1, sampleCount, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY, outImage.mImage, outImage.mAllocation, outImage.mAllocationInfo, errorState))
			return false;

		if (!create2DImageView(renderer.getDevice(), outImage.getImage(), renderer.getDepthFormat(), 1, VK_IMAGE_ASPECT_DEPTH_BIT, outImage.getView(), errorState))
			return false;

		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	// RenderTarget
	//////////////////////////////////////////////////////////////////////////

	SnapshotRenderTarget::SnapshotRenderTarget(Core& core) :
		mRenderService(core.getService<RenderService>())
	{}

	SnapshotRenderTarget::~SnapshotRenderTarget()
	{
		for (auto& frame_buffer : mFramebuffers)
			vkDestroyFramebuffer(mRenderService->getDevice(), frame_buffer, nullptr);
	
		if (mRenderPass != VK_NULL_HANDLE)
			vkDestroyRenderPass(mRenderService->getDevice(), mRenderPass, nullptr);

		destroyImageAndView(mDepthImage, mRenderService->getDevice(), mRenderService->getVulkanAllocator());
		destroyImageAndView(mColorImage, mRenderService->getDevice(), mRenderService->getVulkanAllocator());
	}

	bool SnapshotRenderTarget::init(Snapshot* snapshot, utility::ErrorState& errorState)
	{
		assert(snapshot != nullptr);
		mSnapshot = snapshot;

		mSize = mSnapshot->mCellSize;
		mClearColor = mSnapshot->mClearColor;

		// Warn if requested number of samples is not matched by hardware
		if (!mRenderService->getRasterizationSamples(mSnapshot->mRequestedSamples, mRasterizationSamples, errorState))
			nap::Logger::warn(errorState.toString().c_str());

		// Check if sample rate shading is enabled
		if (mSnapshot->mSampleShading && !(mRenderService->sampleShadingSupported()))
		{
			nap::Logger::warn("Sample shading requested but not supported");
			mSampleShading = false;
		}

		// Assert the cells are created
		assert(mSnapshot->mColorTextures.size() > 0);
		mFormat = mSnapshot->mColorTextures[0]->getFormat();

		// Create a framebuffer for every cell
		int num_cells = mSnapshot->mColorTextures.size();
		mFramebuffers.resize(num_cells);

		// Framebuffer and attachment sizes
		VkExtent2D framebuffer_size = { mSize.x, mSize.y };

		// Create framebuffer info
		VkFramebufferCreateInfo framebuffer_info = {};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.width = framebuffer_size.width;
		framebuffer_info.height = framebuffer_size.height;
		framebuffer_info.layers = 1;

		// Create single or multi-sample renderpass based on rasterization samples
		if (!createSnapshotRenderPass(mRenderService->getDevice(), mFormat, mRenderService->getDepthFormat(), mRasterizationSamples, mRenderPass, errorState))
			return false;

		if (mRasterizationSamples == VK_SAMPLE_COUNT_1_BIT)
		{
			// Create depth attachment
			if (!createDepthResource(*mRenderService, framebuffer_size, mRasterizationSamples, mDepthImage, errorState))
				return false;

			framebuffer_info.attachmentCount = 2;
			framebuffer_info.renderPass = mRenderPass;

			// Bind textures as attachments
			for (int i = 0; i < num_cells; i++) {
				std::array<VkImageView, 2> attachments{ mSnapshot->mColorTextures[i]->getHandle().getView(), mDepthImage.getView() };
				framebuffer_info.pAttachments = attachments.data();

				// Create framebuffer
				if (!errorState.check(vkCreateFramebuffer(mRenderService->getDevice(), &framebuffer_info, nullptr, &mFramebuffers[i]) == VK_SUCCESS, "Failed to create framebuffer"))
					return false;
			}
		}
		else
		{
			// Create multi-sampled color attachment
			if (!createColorResource(*mRenderService, framebuffer_size, mFormat, mRasterizationSamples, mColorImage, errorState))
				return false;

			// Create multi-sampled depth attachment
			if (!createDepthResource(*mRenderService, framebuffer_size, mRasterizationSamples, mDepthImage, errorState))
				return false;

			framebuffer_info.attachmentCount = 3;
			framebuffer_info.renderPass = mRenderPass;

			for (int i = 0; i < num_cells; i++) {
				std::array<VkImageView, 3> attachments{ mColorImage.getView(), mDepthImage.getView(), mSnapshot->mColorTextures[i]->getHandle().getView() };
				framebuffer_info.pAttachments = attachments.data();

				// Create a framebuffer that links the cell target texture to the appropriate resolved color attachment
				if (!errorState.check(vkCreateFramebuffer(mRenderService->getDevice(), &framebuffer_info, nullptr, &mFramebuffers[i]) == VK_SUCCESS, "Failed to create framebuffer"))
					return false;
			}
		}
		return true;
	}


	void SnapshotRenderTarget::beginRendering()
	{
		glm::ivec2 size = getBufferSize();
		const RGBAColorFloat& clear_color = mSnapshot->mClearColor;

		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { clear_color[0], clear_color[1], clear_color[2], clear_color[3] };
		clearValues[1].depthStencil = { 1.0f, 0 };

		// Setup render pass
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = mRenderPass;
		renderPassInfo.framebuffer = mFramebuffers[mCellIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = { mSize.x, mSize.y };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		// Begin render pass
		vkCmdBeginRenderPass(mRenderService->getCurrentCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Ensure scissor and viewport are covering the cell area
		VkRect2D rect = {};
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
	}

	void SnapshotRenderTarget::endRendering()
	{
		vkCmdEndRenderPass(mRenderService->getCurrentCommandBuffer());
	}


	const glm::ivec2 SnapshotRenderTarget::getBufferSize() const
	{
		return static_cast<glm::ivec2>(mSize);
	}


	VkFormat SnapshotRenderTarget::getColorFormat() const
	{
		return mFormat;
	}


	VkFormat SnapshotRenderTarget::getDepthFormat() const
	{
		return mRenderService->getDepthFormat();
	}


	VkSampleCountFlagBits SnapshotRenderTarget::getSampleCount() const
	{
		return mRasterizationSamples;
	}


	bool SnapshotRenderTarget::getSampleShadingEnabled() const
	{
		return mSnapshot->mSampleShading;
	}

} // nap
