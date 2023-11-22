/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "irendertarget.h"
#include "rendertexturecube.h"

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <vulkan/vulkan_core.h>

namespace nap
{
	// Forward Declares
	class TextureCube;
	class DepthRenderTextureCube;
	class RenderService;
	class PerspCameraComponentInstance;

	// Called when rendering a cube depth texture layer
	using CubeDepthRenderTargetCallback = std::function<void(CubeDepthRenderTarget& target, const glm::mat4& projection, const glm::mat4& view)>;


	//////////////////////////////////////////////////////////////////////////
	// CubeDepthRenderTarget
	//////////////////////////////////////////////////////////////////////////

	/**
	 * CubeDepthRenderTarget
	 *
	 * A specialized version of nap::RenderTarget that renders a cube depth texture. As a cube texture comprises
	 * six views of the scene, the `render` function carries out six render passes, each from a different point
	 * of view by means of particular view and projection transformations. The result of these render passes is
	 * stored in a multi-layer Vulkan image, each layer representing one view through the side of a unit cube.
	 */
	class NAPAPI CubeDepthRenderTarget : public Resource, public IRenderTarget
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		 * Every cube depth render target requires a reference to core.
		 * @param core link to a nap core instance
		 */
		CubeDepthRenderTarget(Core& core);
		
		/**
		 * Destroys allocated render resources
		 */
		~CubeDepthRenderTarget();

		/**
		 * Initializes the render target, including all the required resources.
		 * @param errorState contains the error if initialization failed.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Starts a render pass for the current layer index. Called by CubeDepthRenderTarget::renderInternal.
		 */
		virtual void beginRendering() override;

		/**
		 * Ends a render pass for the current layer index. Called by CubeDepthRenderTarget::renderInternal.
		 */
		virtual void endRendering() override;

		/**
		 * @return the size of the buffer in pixels
		 */
		virtual const glm::ivec2 getBufferSize() const override					{ return mSize; }

		/**
		 * Updates the render target clear color.
		 * @param color the new clear color to use.
		 */
		virtual void setClearColor(const RGBAColorFloat& color) override		{ mClearValue = color.getRed(); }
		
		/**
		 * @return the currently used render target clear color.
		 */
		virtual const RGBAColorFloat& getClearColor() const override			{ return mClearColor; }

		/**
		 * Geometry winding order, defaults to clockwise. 
		 */
		virtual ECullWindingOrder getWindingOrder() const override				{ return ECullWindingOrder::Clockwise; }

		/**
		 * @return the render pass
		 */
		virtual VkRenderPass getRenderPass() const override						{ return mRenderPass; }

		/**
		 * @return render target color format.
		 */
		virtual VkFormat getColorFormat() const override						{ return VK_FORMAT_UNDEFINED; }

		/**
		 * @return render target depth format
		 */
		virtual VkFormat getDepthFormat() const override						{ return mVulkanDepthFormat; }

		/**
		 * Cube textures currently only support single sample render pass. Therefore, this function always returns `VK_SAMPLE_COUNT_1_BIT`.
		 * @return used number of samples when rendering to the target.
		 */
		virtual VkSampleCountFlagBits getSampleCount() const override			{ return VK_SAMPLE_COUNT_1_BIT; }
		
		/**
		 * @return if sample based shading is enabled when rendering to the target.
		 */
		virtual bool getSampleShadingEnabled() const override					{ return mSampleShading; }

		/**
		 * @return the absolute size of a single cube face in pixels.
		 */
		glm::ivec2 getSize() const												{ return mSize; }

		/**
		 * Renders objects to nap::DepthRenderTextureCube using the specified nap::CubeRenderTarget. Use 'renderCallback' to
		 * group the rendering work of a single render pass. This pass is repeated six times to create the resulting
		 * cube texture. The view and projection transformations are computed by nap::CubeRenderTarget and passed as
		 * arguments to a callback function. The following example demonstrates this using a lambda function:
		 *
		 * ~~~~~{.cpp}
		 *	if (mRenderService->beginHeadlessRecording())
		 *	{
		 *		cube_target->render([rs = mRenderService, comps = render_comps](CubeDepthRenderTarget& target, const glm::mat4& projection, const glm::mat4& view)
		 *		{
		 *			rs->renderObjects(target, projection, view, comps, std::bind(&sorter::sortObjectsByDepth, std::placeholders::_1, std::placeholders::_2));
		 *		});
		 *		mRenderService->endHeadlessRecording();
		 *	}
		 * ~~~~~
		 */
		void render(CubeDepthRenderTargetCallback renderCallback);

		/**
		 * Renders objects to nap::DepthRenderTextureCube using the specified nap::CubeRenderTarget. Use 'renderCallback' to
		 * group the rendering work of a single render pass. This pass is repeated six times to create the resulting
		 * cube texture. The view and projection transformations are computed by nap::CubeRenderTarget and passed as
		 * arguments to a callback function. The following example demonstrates this using a lambda function:
		 *
		 * ~~~~~{.cpp}
		 *	if (mRenderService->beginHeadlessRecording())
		 *	{
		 *		cube_target->render(*persp_camera, [rs = mRenderService, comps = render_comps](CubeDepthRenderTarget& target, const glm::mat4& projection, const glm::mat4& view)
		 *		{
		 *			rs->renderObjects(target, projection, view, comps, std::bind(&sorter::sortObjectsByDepth, std::placeholders::_1, std::placeholders::_2));
		 *		});
		 *		mRenderService->endHeadlessRecording();
		 *	}
		 * ~~~~~
		 */
		void render(PerspCameraComponentInstance& camera, CubeDepthRenderTargetCallback renderCallback);

		bool									mSampleShading = true;										///< Property: 'SampleShading' Reduces texture aliasing when enabled, at higher computational cost.
		float									mClearValue = 1.0f;											///< Property: 'ClearValue' value selection used for clearing the render target
		ERasterizationSamples					mRequestedSamples = ERasterizationSamples::One;				///< Property: 'Samples' The number of samples used during Rasterization. For better results turn on 'SampleShading'.
		DepthRenderTextureCube::EDepthFormat	mDepthFormat = DepthRenderTextureCube::EDepthFormat::D32;	///< Property: 'DepthFormat' the cube texture depth format.
																											
		ResourcePtr<DepthRenderTextureCube>		mCubeDepthTexture;											///< Property: 'CubeDepthTexture' Cube depth texture to render to.

	private:
		/**
		 * Internal cube texture rendering routine. Called by CubeRenderTarget::render.
		 */
		void renderInternal(const glm::vec3& camPosition, const glm::mat4& projectionMatrix, CubeDepthRenderTargetCallback renderCallback);

		/**
		 * Sets the texture layer index. Called by CubeDepthRenderTarget::renderInternal.
		 */
		void setLayerIndex(uint index);

		RenderService*							mRenderService;
		VkRenderPass							mRenderPass = VK_NULL_HANDLE;
		VkFormat								mVulkanDepthFormat = VK_FORMAT_UNDEFINED;

		std::array<VkFramebuffer, TextureCube::LAYER_COUNT>	mFramebuffers;

		glm::ivec2								mSize;
		RGBAColorFloat							mClearColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		uint									mLayerIndex = 0U;
		bool									mIsFirstPass = true;
	};
}
