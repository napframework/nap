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

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CubeRenderTarget, "Color texture target for cube map render operations")
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("CubeTexture",			&nap::CubeRenderTarget::mCubeTexture,				nap::rtti::EPropertyMetaData::Required, "The storage cube map color texture")
	RTTI_PROPERTY("SampleShading",			&nap::CubeRenderTarget::mSampleShading,				nap::rtti::EPropertyMetaData::Default,	"Reduces texture aliasing at higher computational cost")
	RTTI_PROPERTY("ClearColor",				&nap::CubeRenderTarget::mClearColor,				nap::rtti::EPropertyMetaData::Default,	"Initial clear color")
	RTTI_PROPERTY("UpdateLODs",				&nap::CubeRenderTarget::mUpdateLODs,				nap::rtti::EPropertyMetaData::Default,  "Create mip-maps when texture has more than 1 LOD")
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static
	//////////////////////////////////////////////////////////////////////////

	static const std::vector<glm::mat4> sCubeMapViewMatrices =
	{
		glm::scale(glm::identity<glm::mat4>(), { -1.0f, 1.0f, 1.0f }) * glm::rotate(glm::identity<glm::mat4>(), glm::half_pi<float>(),	math::Y_AXIS),	// right (+X)
		glm::scale(glm::identity<glm::mat4>(), { -1.0f, 1.0f, 1.0f }) * glm::rotate(glm::identity<glm::mat4>(), -glm::half_pi<float>(), math::Y_AXIS),	// left (-X)
		glm::scale(glm::identity<glm::mat4>(), { 1.0f, 1.0f, -1.0f }) * glm::rotate(glm::identity<glm::mat4>(), glm::half_pi<float>(),	math::X_AXIS),	// up (+Y)
		glm::scale(glm::identity<glm::mat4>(), { 1.0f, 1.0f, -1.0f }) * glm::rotate(glm::identity<glm::mat4>(), -glm::half_pi<float>(), math::X_AXIS),	// down (-Y)
		glm::scale(glm::identity<glm::mat4>(), { 1.0f, -1.0f, 1.0f }) * glm::rotate(glm::identity<glm::mat4>(), glm::pi<float>(),		math::X_AXIS),	// back (+Z)
		glm::scale(glm::identity<glm::mat4>(), { -1.0f, 1.0f, 1.0f })																					// forward (-Z)	
	};

	static const std::vector<glm::mat4> sCubeMapInverseViewMatrices =
	{
		glm::inverse(sCubeMapViewMatrices[0]),
		glm::inverse(sCubeMapViewMatrices[1]),
		glm::inverse(sCubeMapViewMatrices[2]),
		glm::inverse(sCubeMapViewMatrices[3]),
		glm::inverse(sCubeMapViewMatrices[4]),
		glm::inverse(sCubeMapViewMatrices[5]),
	};


	// Create the depth image and view
	static bool createLayeredDepthResource(const RenderService& renderer, VkExtent2D targetSize, VkFormat depthFormat, VkSampleCountFlagBits sampleCount, uint layerCount, ImageData& outImage, utility::ErrorState& errorState)
	{
		if (!createLayered2DImage(renderer.getVulkanAllocator(), targetSize.width, targetSize.height, depthFormat, 1, layerCount, sampleCount,
			VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
			outImage.mImage, outImage.mAllocation, outImage.mAllocationInfo, errorState))
			return false;

		if (!createCubeImageView(renderer.getDevice(), outImage.getImage(), depthFormat, 1, VK_IMAGE_ASPECT_DEPTH_BIT, layerCount, outImage.mView, errorState))
			return false;

		for (uint i = 0U; i < layerCount; i++)
		{
			if (!createLayered2DImageView(renderer.getDevice(), outImage.getImage(), depthFormat, 1, VK_IMAGE_ASPECT_DEPTH_BIT, i, 1, outImage.mSubViews[i], errorState))
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
		{
			if (fb != VK_NULL_HANDLE)
				vkDestroyFramebuffer(mRenderService->getDevice(), fb, nullptr);
		}

		if (mRenderPass != VK_NULL_HANDLE)
			vkDestroyRenderPass(mRenderService->getDevice(), mRenderPass, nullptr);

		utility::destroyImageAndView(mColorImage, mRenderService->getDevice(), mRenderService->getVulkanAllocator());
		utility::destroyImageAndView(mDepthImage, mRenderService->getDevice(), mRenderService->getVulkanAllocator());
	}


	bool CubeRenderTarget::init(utility::ErrorState& errorState)
	{
		// Check if sample rate shading is enabled
		if (mSampleShading && !(mRenderService->sampleShadingSupported()))
		{
			nap::Logger::warn("Sample shading requested but not supported");
			mSampleShading = false;
		}

		// Set size
		mSize = { mCubeTexture->getWidth(), mCubeTexture->getHeight() };

		SurfaceDescriptor color_settings = { static_cast<uint32_t>(mSize.x), static_cast<uint32_t>(mSize.y), ESurfaceDataType::BYTE, ESurfaceChannels::RGBA, EColorSpace::Linear };
		mVulkanColorFormat = utility::getTextureFormat(color_settings);
		assert(mVulkanColorFormat != VK_FORMAT_UNDEFINED);

		SurfaceDescriptor depth_settings = color_settings;
		depth_settings.mChannels = ESurfaceChannels::D;
		depth_settings.mDataType = ESurfaceDataType::USHORT;
		mVulkanDepthFormat = utility::getTextureFormat(depth_settings);
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
		if (!createRenderPass(mRenderService->getDevice(), mVulkanColorFormat, mVulkanDepthFormat, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mRenderPass, errorState))
			return false;

		framebuffer_info.renderPass = mRenderPass;

		const auto* cube_texture = static_cast<const TextureCube*>(mCubeTexture.get());
		if (!createLayeredDepthResource(*mRenderService, framebuffer_size, mVulkanDepthFormat, VK_SAMPLE_COUNT_1_BIT, cube_texture->getLayerCount(), mDepthImage, errorState))
			return false;

		for (uint i = 0U; i < cube_texture->getLayerCount(); i++)
		{
			std::array<VkImageView, 2> attachments{ cube_texture->getHandle().getSubView(i), mDepthImage.getSubView(i) };
			framebuffer_info.pAttachments = attachments.data();
			framebuffer_info.attachmentCount = attachments.size();

			// Create framebuffer
			if (!errorState.check(vkCreateFramebuffer(mRenderService->getDevice(), &framebuffer_info, nullptr, &mFramebuffers[i]) == VK_SUCCESS, "Failed to create framebuffer"))
				return false;
		}

		return true;
	}


	void CubeRenderTarget::beginRendering()
	{
		// We transition the layout of the depth attachment from UNDEFINED to DEPTH_STENCIL_ATTACHMENT_OPTIMAL, once in the first pass
		if (mIsFirstPass)
		{
			utility::transitionImageLayout(mRenderService->getCurrentCommandBuffer(), mDepthImage.mImage,
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


	void CubeRenderTarget::render(CubeRenderTargetCallback renderCallback)
	{
		static const auto projection_matrix = glm::perspective(90.0f, 1.0f, 0.01f, 1000.0f);
		renderInternal(glm::zero<glm::vec3>(), projection_matrix, renderCallback);
	}


	void CubeRenderTarget::render(PerspCameraComponentInstance& camera, CubeRenderTargetCallback renderCallback)
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
		renderInternal(cam_position, camera.getProjectionMatrix(), renderCallback);
	}


	void CubeRenderTarget::renderInternal(const glm::vec3& camPosition, const glm::mat4& projectionMatrix, CubeRenderTargetCallback renderCallback)
	{
		/**
		 * Render to frame buffers
		 * Cube face selection following the Vulkan spec
		 * https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap16.html#_cube_map_face_selection_and_transformations
		 **/
		const auto cam_translation = glm::translate(glm::identity<glm::mat4>(), camPosition);
		for (int layer_index = TextureCube::layerCount - 1; layer_index >= 0; layer_index--)
		{
			setLayerIndex(layer_index);
			beginRendering();
			{
				const auto view_matrix = CubeRenderTarget::getCubeMapInverseViewMatrices()[layer_index] * cam_translation;
				renderCallback(*this, projectionMatrix, view_matrix);
			}
			endRendering();
		}

		// Update mip maps
		if (mUpdateLODs && mCubeTexture->getMipLevels() > 1)
		{
			// Check whether the texture is flagged as depth
			bool is_depth = mCubeTexture->getDescriptor().getChannels() == ESurfaceChannels::D;
			VkImageAspectFlags aspect = is_depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

			// Layout transition to TRANSFER_DST to setup the mip map blit operation
			utility::transitionImageLayout(mRenderService->getCurrentCommandBuffer(), mCubeTexture->getHandle().getImage(),
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				0, mCubeTexture->getMipLevels(),
				0, TextureCube::layerCount, VK_IMAGE_ASPECT_COLOR_BIT);

			utility::createMipmaps(mRenderService->getCurrentCommandBuffer(), mCubeTexture->getHandle().getImage(),
				mCubeTexture->getFormat(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, aspect,
				mCubeTexture->getWidth(), mCubeTexture->getHeight(), mCubeTexture->getMipLevels(), 0, TextureCube::layerCount
			);
		}

		mIsFirstPass = false;
	}


	void CubeRenderTarget::setLayerIndex(uint index)
	{
		assert(index < TextureCube::layerCount);
		mLayerIndex = std::clamp(index, 0U, TextureCube::layerCount - 1);
	}


	const std::vector<glm::mat4>& CubeRenderTarget::getCubeMapViewMatrices()
	{
		return sCubeMapViewMatrices;
	}


	const std::vector<glm::mat4>& CubeRenderTarget::getCubeMapInverseViewMatrices()
	{
		return sCubeMapInverseViewMatrices;
	}
}
