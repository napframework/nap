/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "cuberendertarget.h"

// Nap includes
#include <perspcameracomponent.h>
#include <transformcomponent.h>
#include <renderservice.h>
#include <textureutils.h>
#include <nap/core.h>
#include <entity.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CubeRenderTarget)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("CubeTexture",			&nap::CubeRenderTarget::mCubeTexture,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SampleShading",			&nap::CubeRenderTarget::mSampleShading,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Samples",				&nap::CubeRenderTarget::mRequestedSamples,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ClearColor",				&nap::CubeRenderTarget::mClearColor,				nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static functions
	//////////////////////////////////////////////////////////////////////////
	
	// Creates the color image and view
	static bool createLayeredColorResource(const RenderService& renderer, VkExtent2D targetSize, VkFormat colorFormat, VkSampleCountFlagBits sampleCount, uint layerCount, ImageData& outImage, utility::ErrorState& errorState)
	{
		if (!createLayered2DImage(renderer.getVulkanAllocator(), targetSize.width, targetSize.height, colorFormat, layerCount, sampleCount,
			VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
			outImage.mImage, outImage.mAllocation, outImage.mAllocationInfo, errorState))
			return false;

		if (!createCubeImageView(renderer.getDevice(), outImage.getImage(), colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, layerCount, outImage.mView, errorState))
			return false;

		return true;
	}


	// Create the depth image and view
	static bool createLayeredDepthResource(const RenderService& renderer, VkExtent2D targetSize, VkFormat depthFormat, VkSampleCountFlagBits sampleCount, uint layerCount, ImageData& outImage, utility::ErrorState& errorState)
	{
		if (!createLayered2DImage(renderer.getVulkanAllocator(), targetSize.width, targetSize.height, depthFormat, layerCount, sampleCount,
			VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
			outImage.mImage, outImage.mAllocation, outImage.mAllocationInfo, errorState))
			return false;

		if (!createCubeImageView(renderer.getDevice(), outImage.getImage(), depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, layerCount, outImage.mView, errorState))
			return false;

		for (uint i = 0U; i < layerCount; i++)
		{
			if (!createLayered2DImageView(renderer.getDevice(), outImage.getImage(), depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, i, 1, outImage.getSubView(i), errorState))
				return false;
		}

		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	// CubeRenderTarget
	//////////////////////////////////////////////////////////////////////////

	CubeRenderTarget::CubeRenderTarget(Core& core) :
		mRenderService(core.getService<RenderService>())
	{}


	CubeRenderTarget::~CubeRenderTarget()
	{
		for (auto& fb : mFramebuffers)
			vkDestroyFramebuffer(mRenderService->getDevice(), fb, nullptr);

		if (mRenderPass != nullptr)
			vkDestroyRenderPass(mRenderService->getDevice(), mRenderPass, nullptr);

		destroyImageAndView(mColorImage, mRenderService->getDevice(), mRenderService->getVulkanAllocator());
		destroyImageAndView(mDepthImage, mRenderService->getDevice(), mRenderService->getVulkanAllocator());
	}


	bool CubeRenderTarget::init(utility::ErrorState& errorState)
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

		// Set size
		mSize = { mCubeTexture->getWidth(), mCubeTexture->getHeight() };

		SurfaceDescriptor color_settings = { static_cast<uint32_t>(mSize.x), static_cast<uint32_t>(mSize.y), ESurfaceDataType::BYTE, ESurfaceChannels::RGBA, EColorSpace::Linear };
		mVulkanColorFormat = getTextureFormat(color_settings);
		assert(mVulkanColorFormat != VK_FORMAT_UNDEFINED);

		SurfaceDescriptor depth_settings = color_settings;
		depth_settings.mChannels = ESurfaceChannels::D;
		depth_settings.mDataType = ESurfaceDataType::USHORT;
		mVulkanDepthFormat = getTextureFormat(depth_settings);
		assert(mVulkanDepthFormat != VK_FORMAT_UNDEFINED);

		// Framebuffer and attachment sizes
		VkExtent2D framebuffer_size = { static_cast<uint32_t>(mSize.x), static_cast<uint32_t>(mSize.y) };

		// Create framebuffer info
		VkFramebufferCreateInfo framebuffer_info = {};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.width = framebuffer_size.width;
		framebuffer_info.height = framebuffer_size.height;
		framebuffer_info.layers = 1;

		// Create render pass based on number of multi samples
		// When there's only 1 there's no need for a resolve step
		if (!createRenderPass(mRenderService->getDevice(), mVulkanColorFormat, mVulkanDepthFormat, mRasterizationSamples, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mRenderPass, errorState))
			return false;

		if (mRasterizationSamples == VK_SAMPLE_COUNT_1_BIT)
		{
			const auto& cube_texture = static_cast<const TextureCube&>(*mCubeTexture);
			if (!createLayeredDepthResource(*mRenderService, framebuffer_size, mVulkanDepthFormat, mRasterizationSamples, cube_texture.getLayerCount(), mDepthImage, errorState))
				return false;

			for (uint i = 0U; i < cube_texture.getLayerCount(); i++)
			{
				std::array<VkImageView, 2> attachments{ cube_texture.getHandle().getSubView(i), mDepthImage.getSubView(i) };
				framebuffer_info.pAttachments = attachments.data();
				framebuffer_info.attachmentCount = attachments.size();
				framebuffer_info.renderPass = mRenderPass;

				// Create framebuffer
				if (!errorState.check(vkCreateFramebuffer(mRenderService->getDevice(), &framebuffer_info, nullptr, &mFramebuffers[i]) == VK_SUCCESS, "Failed to create framebuffer"))
					return false;
			}
		}
		else
		{
			NAP_ASSERT_MSG(false, "Multisampled cube render targets not yet implemented");
			//// Create multi-sampled color attachments
			//for (auto& img : mColorImages)
			//{
			//	if (!createColorResource(*mRenderService, framebuffer_size, getTextureFormat(color_settings), mRasterizationSamples, img, errorState))
			//		return false;
			//}

			//// Create multi-sampled depth attachment
			//if (!createDepthResource(*mRenderService, framebuffer_size, mRasterizationSamples, mDepthImage, errorState))
			//	return false;

			//std::array<VkImageView, 3> attachments{ mColorImage.mTextureView, mDepthImage.mTextureView, mColorTexture->getImageView() };
			//framebuffer_info.pAttachments = attachments.data();
			//framebuffer_info.attachmentCount = attachments.size();
			//framebuffer_info.renderPass = mRenderPass;

			//// Create a framebuffer that links the cell target texture to the appropriate resolved color attachment
			//if (!errorState.check(vkCreateFramebuffer(mRenderService->getDevice(), &framebuffer_info, nullptr, &mFramebuffers[i]) == VK_SUCCESS, "Failed to create framebuffer"))
			//	return false;
		}

		return true;
	}


	void CubeRenderTarget::beginRendering()
	{
		// We transition the layout of the depth attachment from UNDEFINED to DEPTH_STENCIL_ATTACHMENT_OPTIMAL, once in the first pass
		if (mIsFirstPass)
		{
			transitionImageLayout(mRenderService->getCurrentCommandBuffer(), mDepthImage.mImage,
				mDepthImage.mCurrentLayout, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				0, 1, VK_IMAGE_ASPECT_DEPTH_BIT);
		}

		const RGBAColorFloat& clear_color = mClearColor;

		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { clear_color[0], clear_color[1], clear_color[2], clear_color[3] };
		clearValues[1].depthStencil = { 1.0f, 0 };

		const glm::ivec2 offset = { 0, 0 };

		// Setup render pass
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = mRenderPass;
		renderPassInfo.framebuffer = mFramebuffers[mLayerIndex];
		renderPassInfo.renderArea.offset = { offset.x, offset.y };
		renderPassInfo.renderArea.extent = { static_cast<uint32_t>(mSize.x), static_cast<uint32_t>(mSize.y) };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		// Begin render pass
		vkCmdBeginRenderPass(mRenderService->getCurrentCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Ensure scissor and viewport are covering the cell area
		VkRect2D rect = {};
		rect.offset = { offset.x, offset.y };
		rect.extent = { static_cast<uint32_t>(mSize.x), static_cast<uint32_t>(mSize.y) };
		vkCmdSetScissor(mRenderService->getCurrentCommandBuffer(), 0, 1, &rect);

		VkViewport viewport = {};
		viewport.x = static_cast<float>(offset.x);
		viewport.y = mSize.y + static_cast<float>(offset.y);
		viewport.width = mSize.x;
		viewport.height = -mSize.y;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(mRenderService->getCurrentCommandBuffer(), 0, 1, &viewport);
	}


	void CubeRenderTarget::endRendering()
	{
		vkCmdEndRenderPass(mRenderService->getCurrentCommandBuffer());
	}


	void CubeRenderTarget::render(PerspCameraComponentInstance& camera, std::function<void(CubeRenderTarget&, const glm::mat4& projection, const glm::mat4& view)> renderCallback)
	{
		// Update camera properties
		camera.setFieldOfView(90.0f);
		camera.setGridLocation(0, 0);
		camera.setGridDimensions(1, 1);
		camera.setRenderTargetSize(mSize);

		// Fetch camera transform
		auto& cam_trans = camera.getEntityInstance()->getComponent<TransformComponentInstance>();
		const auto& cam_position = math::extractPosition(cam_trans.getGlobalTransform());

		// Render
		render(cam_position, camera.getProjectionMatrix(), renderCallback);
	}


	void CubeRenderTarget::render(const glm::vec3& camPosition, const glm::mat4& projectionMatrix, std::function<void(CubeRenderTarget&, const glm::mat4& projection, const glm::mat4& view)> renderCallback)
	{
		/**
		 * Render to frame buffers
		 * Cube face selection following the Vulkan spec
		 * https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap16.html#_cube_map_face_selection_and_transformations
		 **/
		const auto cam_translation = glm::translate(glm::identity<glm::mat4>(), camPosition);

		setLayerIndex(5);
		beginRendering();
		{
			// forward (-Z)
			const auto trans = glm::scale(glm::identity<glm::mat4>(), { -1.0f, 1.0f, 1.0f });
			auto view = glm::inverse(cam_translation * trans);
			renderCallback(*this, projectionMatrix, view);
		}
		endRendering();

		setLayerIndex(4);
		beginRendering();
		{
			// back (+Z)
			const auto trans = glm::scale(glm::identity<glm::mat4>(), { 1.0f, -1.0f, 1.0f }) * glm::rotate(glm::identity<glm::mat4>(), glm::pi<float>(), math::X_AXIS);
			auto view = glm::inverse(cam_translation * trans);
			renderCallback(*this, projectionMatrix, view);
		}
		endRendering();

		setLayerIndex(3);
		beginRendering();
		{
			// down (-Y)
			const auto trans = glm::scale(glm::identity<glm::mat4>(), { 1.0f, 1.0f, -1.0f }) * glm::rotate(glm::identity<glm::mat4>(), -glm::half_pi<float>(), math::X_AXIS);
			auto view = glm::inverse(cam_translation * trans);
			renderCallback(*this, projectionMatrix, view);
		}
		endRendering();

		setLayerIndex(2);
		beginRendering();
		{
			// up (+Y)
			const auto trans = glm::scale(glm::identity<glm::mat4>(), { 1.0f, 1.0f, -1.0f }) * glm::rotate(glm::identity<glm::mat4>(), glm::half_pi<float>(), math::X_AXIS);
			auto view = glm::inverse(cam_translation * trans);
			renderCallback(*this, projectionMatrix, view);
		}
		endRendering();

		setLayerIndex(1);
		beginRendering();
		{
			// left (-X)
			const auto trans = glm::scale(glm::identity<glm::mat4>(), { -1.0f, 1.0f, 1.0f }) * glm::rotate(glm::identity<glm::mat4>(), -glm::half_pi<float>(), math::Y_AXIS);
			auto view = glm::inverse(cam_translation * trans);
			renderCallback(*this, projectionMatrix, view);
		}
		endRendering();

		setLayerIndex(0);
		beginRendering();
		{
			// right (+X)
			const auto trans = glm::scale(glm::identity<glm::mat4>(), { -1.0f, 1.0f, 1.0f }) * glm::rotate(glm::identity<glm::mat4>(), glm::half_pi<float>(), math::Y_AXIS);
			auto view = glm::inverse(cam_translation * trans);
			renderCallback(*this, projectionMatrix, view);
		}
		endRendering();

		mIsFirstPass = false;
	}


	void CubeRenderTarget::setLayerIndex(uint index)
	{
		assert(index < TextureCube::LAYER_COUNT);
		mLayerIndex = std::clamp(index, 0U, TextureCube::LAYER_COUNT - 1);
	}
}
