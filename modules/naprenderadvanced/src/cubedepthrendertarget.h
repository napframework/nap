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

	//////////////////////////////////////////////////////////////////////////
	// Quilt RenderTarget
	//////////////////////////////////////////////////////////////////////////

	/**
	 * CubeDepthRenderTarget
	 *
	 * A specialized version of nap::RenderTarget that renders a quilt texture, comprising a number of views of one or
	 * multiple objects in a single nap::RenderTexture2D. The layout of the quilt is determined by the nap::QuiltSettings
	 * acquired from the nap::LookingGlassDevice resource in the "Device" property. 
	 *
	 * When rendering, the perspective shift between views is handled automatically by nap::CubeDepthRenderTarget and 
	 * specified nap::QuiltCameraComponentInstance when using the function CubeDepthRenderTarget::render(). This is 
	 * necessary as the rendering of multiple views requires the setup of a renderpass for each view, as opposed to 
	 * regular use of nap::RenderTarget or nap::RenderWindow. Refer to the following example:
	 *
	 * ~~~~~{.cpp} 
	 *	if (mRenderService->beginHeadlessRecording())
	 *	{
	 *		quilt_target->render(quilt_camera, [render_service = mRenderService, comps = render_comps](CubeDepthRenderTarget& target, QuiltCameraComponentInstance& camera)
	 *		{
	 *			render_service->renderObjects(target, camera, comps);
	 *		});
	 *		mRenderService->endHeadlessRecording();
	 *	}
	 * ~~~~~
	 */
	class NAPAPI CubeDepthRenderTarget : public Resource, public IRenderTarget
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		 * Every render target requires a reference to core.
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
		 * Starts the render pass. Called by CubeDepthRenderTarget::render().
		 */
		virtual void beginRendering() override;

		/**
		 * Ends the render pass. Called by CubeDepthRenderTarget::render().
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
		 * @return used number of samples when rendering to the target.
		 */
		virtual VkSampleCountFlagBits getSampleCount() const override			{ return mRasterizationSamples; }
		
		/**
		 * @return if sample based shading is enabled when rendering to the target.
		 */
		virtual bool getSampleShadingEnabled() const override					{ return mSampleShading; }

		/**
		 * @return the absolute size of a single cube face in pixels.
		 */
		glm::ivec2 getSize() const												{ return mSize; }

		/**
		 * Renders a quilt to nap::RenderTexture2D using the specified nap::CubeDepthRenderTarget. Use 'renderCallback' to
		 * group the rendering work of a single render pass. This pass is repeated a number of times to create the 
		 * resulting quilt texture. The perspective shift between views is handled automatically.
		 * Call this function as follows:
		 *
		 * ~~~~~{.cpp} 
		 *	if (mRenderService->beginHeadlessRecording())
		 *	{
		 *		quilt_target->render(quilt_camera, [render_service = mRenderService, comps = render_comps](CubeDepthRenderTarget& target, QuiltCameraComponentInstance& camera)
		 *		{
		 *			render_service->renderObjects(target, camera, comps);
		 *		});
		 *		mRenderService->endHeadlessRecording();
		 *	}
		 * ~~~~~
		 */
		void render(PerspCameraComponentInstance& camera, std::function<void(CubeDepthRenderTarget&, const glm::mat4& projection, const glm::mat4& view)> renderCallback);

		/**
		 * 
		 */
		void setLayerIndex(uint index);

		bool									mSampleShading = true;										///< Property: 'SampleShading' Reduces texture aliasing when enabled, at higher computational cost.
		float									mClearValue = 1.0f;											///< Property: 'ClearValue' value selection used for clearing the render target
		ERasterizationSamples					mRequestedSamples = ERasterizationSamples::One;				///< Property: 'Samples' The number of samples used during Rasterization. For better results turn on 'SampleShading'.
		DepthRenderTextureCube::EDepthFormat	mDepthFormat = DepthRenderTextureCube::EDepthFormat::D32;	///< Property: 'DepthFormat'

		ResourcePtr<DepthRenderTextureCube>		mCubeDepthTexture;											///< Property: 'CubeTexture' Cube texture to render to.

	private:
		RenderService*							mRenderService;
		VkRenderPass							mRenderPass = VK_NULL_HANDLE;
		VkSampleCountFlagBits					mRasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		VkFormat								mVulkanDepthFormat = VK_FORMAT_UNDEFINED;

		std::array<VkFramebuffer, TextureCube::LAYER_COUNT>	mFramebuffers;

		glm::ivec2								mSize;
		RGBAColorFloat							mClearColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		uint									mLayerIndex = 0U;
		bool									mIsFirstPass = true;
	};
}
