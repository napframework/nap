/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "irendertarget.h"

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <vulkan/vulkan_core.h>

namespace nap
{
	// Forward Declares
	class DepthRenderTexture2D;
	class RenderService;

	/**
	 * A resource that is used to render one or multiple objects to nap::DepthRenderTexture2D exclusively.
	 * Usage of this target creates a graphics pipeline that skips the fragment shader stage of all material instances. 
	 * 
	 * This objects requires a link to a nap::DepthRenderTexture2D to store the result of the render pass.
	 * Only render to a render target within a headless recording pass, failure to do so will result in undefined behavior.
	 * Make sure to call beginRendering() to start the render pass and endRendering() to end the render pass.
	 * Always call RenderService::endHeadlessRecording after having recorded all off-screen render operations.
	 *
	 * ~~~~~{.cpp} 
	 *		mRenderService->beginFrame();
	 *		if (mRenderService->beginHeadlessRecording())
	 *		{
	 *			...
	 *			mTargetOne->beginRendering();
	 *			mRenderService->renderObjects(*mTargetOne, ortho_cam, objects_one);
	 *			mTargetOne->endRendering();
	 *			...
	 *			mTargetTwo->beginRendering();
	 *			mRenderService->renderObjects(*mTargetTwo, ortho_cam, objects_two);
	 *			mTargetTwo->endRendering();
	 *			...
	 *			mRenderService->endHeadlessRecording();
	 *		}
	 *		mRenderService->endFrame();
	 * ~~~~~
	 *
	 */
	class NAPAPI DepthRenderTarget : public Resource, public IRenderTarget
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		 * Every render target requires a reference to core.
		 * @param core link to a nap core instance
		 */
		DepthRenderTarget(Core& core);
		
		/**
		 * Destroys allocated render resources
		 */
		~DepthRenderTarget();

		/**
		 * Initializes the render target, including all the required resources.
		 * @param errorState contains the error if initialization failed.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Starts the render pass.
		 * Only start the render pass after a successful call to RenderService::beginHeadlessRecording().
		 *
		 * ~~~~~{.cpp}
		 *		mRenderService->beginFrame();
		 *		if (mRenderService->beginHeadlessRecording())
		 *		{
		 *			...
		 *			mTarget->beginRendering();
		 *			mRenderService->renderObjects(*mTarget, ortho_cam, objects_one);
		 *			mTarget->endRendering();
		 *			...
		 *			mRenderService->endHeadlessRecording();
		 *		}
		 *		mRenderService->endFrame();
		 * ~~~~~
		 */
		virtual void beginRendering() override;

		/**
		 * Ends the render pass. Always call this after beginRendering().
		 *
		 * ~~~~~{.cpp}
		 *		mRenderService->beginFrame();
		 *		if (mRenderService->beginHeadlessRecording())
		 *		{
		 *			...
		 *			mTarget->beginRendering();
		 *			mTarget->renderObjects(*mTarget, ortho_cam, objects_one);
		 *			mTarget->endRendering();
		 *			...
		 *			mRenderService->endHeadlessRecording();
		 *		}
		 *		mRenderService->endFrame();
		 * ~~~~~
		*/
		virtual void endRendering() override;

		/**
		 * @return size in pixels of the render target.
		 */
		virtual const glm::ivec2 getBufferSize() const override;

		/**
		 * Updates the render target clear value. Stores the red component of `color` as the clear value.
		 * @param color the new clear value to use.
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
		 * @return used number of samples when rendering to the target.
		 */
		virtual VkSampleCountFlagBits getSampleCount() const override			{ return VK_SAMPLE_COUNT_1_BIT; }

		/**
		 * @return if sample based shading is enabled when rendering to the target.
		 */
		virtual bool getSampleShadingEnabled() const override					{ return false; };

		/**
		 * @return VK_FORMAT_UNDEFINED as the depth render target has no color attachment.
		 */
		virtual VkFormat getColorFormat() const override						{ return VK_FORMAT_UNDEFINED; }

		/**
		 * @return render target depth format
		 */
		virtual VkFormat getDepthFormat() const override;

		/**
		 * @return the texture that holds the result of the render pass.
		 */
		DepthRenderTexture2D& getDepthTexture();

	public:
		float								mClearValue = 1.0f;									///< Property: 'ClearValue' value selection used for clearing the render target
		bool								mSampleShading = true;								///< Property: 'SampleShading' Reduces texture aliasing when enabled, at higher computational cost.
		ERasterizationSamples				mRequestedSamples = ERasterizationSamples::One;		///< Property: 'Samples' The number of samples used during Rasterization. For better results turn on 'SampleShading'.
		ResourcePtr<DepthRenderTexture2D>	mDepthTexture;										///< Property: 'DepthTexture' depth texture to render to

	private:
		RenderService*						mRenderService;
		VkFramebuffer						mFramebuffer = VK_NULL_HANDLE;
		VkRenderPass						mRenderPass = VK_NULL_HANDLE;
		VkSampleCountFlagBits				mRasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		RGBAColorFloat						mClearColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	};
}
