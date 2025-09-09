/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "cubedepthrendertarget.h"
#include "cuberendertarget.h"
#include "renderadvancedutils.h"

// Nap includes
#include <perspcameracomponent.h>
#include <transformcomponent.h>
#include <renderservice.h>
#include <textureutils.h>
#include <nap/core.h>
#include <entity.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CubeDepthRenderTarget, "Depth texture target for cube map render operations")
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("CubeDepthTexture",		&nap::CubeDepthRenderTarget::mCubeDepthTexture,			nap::rtti::EPropertyMetaData::Required, "Cube depth texture to render to")
	RTTI_PROPERTY("SampleShading",			&nap::CubeDepthRenderTarget::mSampleShading,			nap::rtti::EPropertyMetaData::Default,	"Reduces texture aliasing at higher computational cost")
	RTTI_PROPERTY("Samples",				&nap::CubeDepthRenderTarget::mRequestedSamples,			nap::rtti::EPropertyMetaData::Default,	"The number of MSAA samples to use")
	RTTI_PROPERTY("ClearValue",				&nap::CubeDepthRenderTarget::mClearValue,				nap::rtti::EPropertyMetaData::Default,	"Depth target initial clear value")
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// CubeDepthRenderTarget
	//////////////////////////////////////////////////////////////////////////

	CubeDepthRenderTarget::CubeDepthRenderTarget(Core& core) :
		mRenderService(core.getService<RenderService>())
	{}


	CubeDepthRenderTarget::~CubeDepthRenderTarget()
	{
		for (auto& fb : mFramebuffers)
			vkDestroyFramebuffer(mRenderService->getDevice(), fb, nullptr);

		if (mRenderPass != VK_NULL_HANDLE)
			vkDestroyRenderPass(mRenderService->getDevice(), mRenderPass, nullptr);
	}


	bool CubeDepthRenderTarget::init(utility::ErrorState& errorState)
	{
		// Check if sample rate shading is enabled
		if (mSampleShading && !(mRenderService->sampleShadingSupported()))
		{
			nap::Logger::warn("Sample shading requested but not supported");
			mSampleShading = false;
		}

		// Assign clear color
		float clear_value = std::clamp(mClearValue, 0.0f, 1.0f);
		mClearColor = { clear_value, clear_value, clear_value, clear_value };

		// Set size
		mSize = { mCubeDepthTexture->getWidth(), mCubeDepthTexture->getHeight() };

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
		if (!createDepthOnlyRenderPass(mRenderService->getDevice(), mCubeDepthTexture->getFormat(), getSampleCount(), getFinalLayout(), mRenderPass, errorState))
			return false;

		const auto& tex = static_cast<const DepthRenderTextureCube&>(*mCubeDepthTexture);
		for (uint i = 0U; i < tex.getHandle().getSubViewCount(); i++)
		{
			VkImageView attachment = tex.getHandle().getSubView(i);
			framebuffer_info.pAttachments = &attachment;
			framebuffer_info.attachmentCount = 1;
			framebuffer_info.renderPass = mRenderPass;

			// Create framebuffer
			if (!errorState.check(vkCreateFramebuffer(mRenderService->getDevice(), &framebuffer_info, nullptr, &mFramebuffers[i]) == VK_SUCCESS, "Failed to create framebuffer"))
				return false;
		}

		return true;
	}


	void CubeDepthRenderTarget::beginRendering()
	{
		VkClearValue clear_value = {};
		clear_value.depthStencil = { mClearValue, 0 };

		const glm::ivec2 offset = { 0, 0 };

		// Setup render pass
		VkRenderPassBeginInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass = mRenderPass;
		render_pass_info.framebuffer = mFramebuffers[mLayerIndex];
		render_pass_info.renderArea.offset = { offset.x, offset.y };
		render_pass_info.renderArea.extent = { static_cast<uint32_t>(mSize.x), static_cast<uint32_t>(mSize.y) };
		render_pass_info.clearValueCount = 1U;
		render_pass_info.pClearValues = &clear_value;

		// Begin render pass
		vkCmdBeginRenderPass(mRenderService->getCurrentCommandBuffer(), &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

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


	void CubeDepthRenderTarget::endRendering()
	{
		vkCmdEndRenderPass(mRenderService->getCurrentCommandBuffer());
	}


	void CubeDepthRenderTarget::render(CubeDepthRenderTargetCallback renderCallback)
	{
		static const auto projection_matrix = glm::perspective(90.0f, 1.0f, 0.01f, 1000.0f);
		renderInternal(glm::zero<glm::vec3>(), projection_matrix, renderCallback);
	}


	void CubeDepthRenderTarget::render(PerspCameraComponentInstance& camera, CubeDepthRenderTargetCallback renderCallback)
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


	void CubeDepthRenderTarget::renderInternal(const glm::vec3& camPosition, const glm::mat4& projectionMatrix, CubeDepthRenderTargetCallback renderCallback)
	{
		/**
		 * Render to frame buffers
		 * Cube face selection following the Vulkan spec
		 * https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap16.html#_cube_map_face_selection_and_transformations
		 **/
		const auto cam_translation = glm::translate(glm::identity<glm::mat4>(), -camPosition);

		for (int layer_index = TextureCube::layerCount - 1; layer_index >= 0; layer_index--)
		{
			setLayerIndex(layer_index);
			beginRendering();
			{
				const auto view_matrix = CubeRenderTarget::getCubeMapViewTransforms()[layer_index] * cam_translation;
				renderCallback(*this, projectionMatrix, view_matrix);
			}
			endRendering();
		}
	}


	void CubeDepthRenderTarget::setLayerIndex(uint index)
	{
		assert(index < TextureCube::layerCount);
		mLayerIndex = std::clamp(index, 0U, TextureCube::layerCount - 1);
	}
}
