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

			VkAttachmentDescription color_attachment = {
				.format = colorFormat,
				.samples = samples,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = !multi_sample ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			};

			VkAttachmentDescription depth_attachment = {
				.format = depthFormat,
				.samples = samples,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			};

			VkAttachmentReference color_attachment_ref = {
				.attachment = 0,
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			};

			VkAttachmentReference depth_attachment_ref = {
				.attachment = 1,
				.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			};

			VkSubpassDescription subpass = {
				.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = 1,
				.pColorAttachments = &color_attachment_ref,
				.pDepthStencilAttachment = &depth_attachment_ref
			};

			// WAW (write-after-write) hazard
			// This render pass does not read output from the previous render pass, but a memory dependency is still required to sync writes
			VkSubpassDependency dependency = {
				.srcSubpass = VK_SUBPASS_EXTERNAL,
				.dstSubpass = 0
			};

			// The set of pipeline stages responsible for producing and consuming the color/depth attachments
			dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

			// The types of memory access that are involved with the src and dst stage masks
			dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			dependency.dependencyFlags = 0;

			VkRenderPassCreateInfo renderpass_info = {
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
				.subpassCount = 1,
				.pSubpasses = &subpass,
				.dependencyCount = 1,
				.pDependencies = &dependency
			};

			// Single-sample render pass
			if (!multi_sample)
			{
				std::array<VkAttachmentDescription, 2> attachments = { color_attachment, depth_attachment };
				renderpass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
				renderpass_info.pAttachments = attachments.data();

				return errorState.check(vkCreateRenderPass(device, &renderpass_info, nullptr, &renderPass) == VK_SUCCESS, "Failed to create render pass");
			}

			// Multi-sample render pass
			VkAttachmentDescription resolve_attachment = {
				.format = colorFormat,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};

			VkAttachmentReference resolve_attachment_ref{
				.attachment = 2,
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			};

			subpass.pResolveAttachments = &resolve_attachment_ref;

			std::array<VkAttachmentDescription, 3> attachments = { color_attachment, depth_attachment, resolve_attachment };
			renderpass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
			renderpass_info.pAttachments = attachments.data();

			return errorState.check(vkCreateRenderPass(device, &renderpass_info, nullptr, &renderPass) == VK_SUCCESS, "Failed to create multi-sample render pass");
		}
	}


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
	static bool createDepthResource(const RenderService& renderer, VkExtent2D targetSize, VkSampleCountFlagBits sampleCount, ImageData& outImage, utility::ErrorState& errorState)
	{
		if (!create2DImage(renderer.getVulkanAllocator(), targetSize.width, targetSize.height, renderer.getDepthFormat(), 1, sampleCount, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY, outImage.mImage, outImage.mAllocation, outImage.mAllocationInfo, errorState))
			return false;

		if (!create2DImageView(renderer.getDevice(), outImage.getImage(), renderer.getDepthFormat(), 1, VK_IMAGE_ASPECT_DEPTH_BIT, outImage.mView, errorState))
			return false;

		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	// RenderTarget
	//////////////////////////////////////////////////////////////////////////

	SnapshotRenderTarget::SnapshotRenderTarget(Core& core) :
		mRenderService(core.getService<RenderService>()),
		mTextureLink(*this)
	{}

	SnapshotRenderTarget::~SnapshotRenderTarget()
	{
		for (auto& frame_buffer : mFramebuffers)
			vkDestroyFramebuffer(mRenderService->getDevice(), frame_buffer, nullptr);
	
		if (mRenderPass != VK_NULL_HANDLE)
			vkDestroyRenderPass(mRenderService->getDevice(), mRenderPass, nullptr);

		utility::destroyImageAndView(mDepthImage, mRenderService->getDevice(), mRenderService->getVulkanAllocator());
		utility::destroyImageAndView(mColorImage, mRenderService->getDevice(), mRenderService->getVulkanAllocator());
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
		assert(!mSnapshot->mColorTextures.empty());
		mFormat = mSnapshot->mColorTextures[0]->getFormat();

		// Create a framebuffer for every cell
		int num_cells = static_cast<int>(mSnapshot->mColorTextures.size());
		mFramebuffers.resize(num_cells);

		// Framebuffer and attachment sizes
		VkExtent2D framebuffer_size = { mSize.x, mSize.y };

		// Create framebuffer info
		VkFramebufferCreateInfo framebuffer_info = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.width = framebuffer_size.width,
			.height = framebuffer_size.height,
			.layers = 1
		};

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
			for (int i = 0; i < num_cells; i++)
            {
				std::array<VkImageView, 2> attachments{ std::as_const(*mSnapshot->mColorTextures[i]).getHandle().getView(), mDepthImage.getView() };
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

			for (int i = 0; i < num_cells; i++)
            {
				std::array<VkImageView, 3> attachments{ mColorImage.getView(), mDepthImage.getView(), std::as_const(*mSnapshot->mColorTextures[i]).getHandle().getView() };
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
		const RGBAColorFloat& clear_color = mSnapshot->mClearColor;

		std::array<VkClearValue, 3> clearValues = {};
		clearValues[0].color = { clear_color[0], clear_color[1], clear_color[2], clear_color[3] };
		clearValues[1].depthStencil = { 1.0f, 0 };
		clearValues[2].color = { clear_color[0], clear_color[1], clear_color[2], clear_color[3] };

		// Setup render pass
		VkRenderPassBeginInfo render_pass_info = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = mRenderPass,
			.framebuffer = mFramebuffers[mCellIndex],
			.renderArea = {
				.offset = { 0, 0 },
				.extent = { mSize.x, mSize.y }
			},
			.clearValueCount = static_cast<uint32_t>(clearValues.size()),
			.pClearValues = clearValues.data()
		};

		// Begin render pass
		vkCmdBeginRenderPass(mRenderService->getCurrentCommandBuffer(), &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

		// Ensure scissor and viewport are covering the cell area
		VkRect2D rect = {
			.offset = { 0, 0 },
			.extent = { mSize.x, mSize.y }
		};
		vkCmdSetScissor(mRenderService->getCurrentCommandBuffer(), 0, 1, &rect);

		auto size = glm::vec2(mSize);
		VkViewport viewport = {
			.x = 0.0f,
			.y = size.y,
			.width = size.x,
			.height = -size.y,
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		};
		vkCmdSetViewport(mRenderService->getCurrentCommandBuffer(), 0, 1, &viewport);
	}


	void SnapshotRenderTarget::endRendering()
	{
		// Finalize rendering and sync layout
		vkCmdEndRenderPass(mRenderService->getCurrentCommandBuffer());
		for (auto& tex : mSnapshot->mColorTextures)
			mTextureLink.sync(*tex);
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
