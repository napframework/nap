/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "irendertarget.h"
#include "rendertexture2d.h"
#include "lookingglassdevice.h"
#include "quiltsettings.h"

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>

namespace nap
{
	// Forward Declares
	class RenderTexture2D;
	class RenderService;
	class QuiltCameraComponentInstance;

	//////////////////////////////////////////////////////////////////////////
	// Quilt RenderTarget
	//////////////////////////////////////////////////////////////////////////

	/**
	 * QuiltRenderTarget
	 *
	 * A specialized version of nap::RenderTarget that renders a quilt texture, comprising a number of views of one or
	 * multiple objects in a single nap::RenderTexture2D. The layout of the quilt is determined by the nap::QuiltSettings
	 * acquired from the nap::LookingGlassDevice resource in the "Device" property. 
	 *
	 * When rendering, the perspective shift between views is handled automatically by nap::QuiltRenderTarget and 
	 * specified nap::QuiltCameraComponentInstance when using the function QuiltRenderTarget::render(). This is 
	 * necessary as the rendering of multiple views requires the setup of a renderpass for each view, as opposed to 
	 * regular use of nap::RenderTarget or nap::RenderWindow. Refer to the following example:
	 *
	 * ~~~~~{.cpp} 
	 *	if (mRenderService->beginHeadlessRecording())
	 *	{
	 *		quilt_target->render(quilt_camera, [render_service = mRenderService, comps = render_comps](QuiltRenderTarget& target, QuiltCameraComponentInstance& camera)
	 *		{
	 *			render_service->renderObjects(target, camera, comps);
	 *		});
	 *		mRenderService->endHeadlessRecording();
	 *	}
	 * ~~~~~
	 */
	class NAPAPI QuiltRenderTarget : public Resource, public IRenderTarget
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		 * Every render target requires a reference to core.
		 * @param core link to a nap core instance
		 */
		QuiltRenderTarget(Core& core);
		
		/**
		 * Destroys allocated render resources
		 */
		~QuiltRenderTarget();

		/**
		 * Initializes the render target, including all the required resources.
		 * @param errorState contains the error if initialization failed.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	private:
		/**
		 * Starts the render pass. Called by QuiltRenderTarget::render().
		 */
		virtual void beginRendering() override;

		/**
		 * Ends the render pass. Called by QuiltRenderTarget::render().
		*/
		virtual void endRendering() override;

	public:
		/**
		 * This function somewhat unintuitively returns the resolution of the looking glass device. This however ensures the
		 * camera aspect ratio always matches that of the monitor. See the function RenderService::renderObjects().
		 * @return the resolution of the looking glass device.
		 */
		virtual const glm::ivec2 getBufferSize() const override					{ return mDevice->getResolution(); }

		/**
		 * Updates the render target clear color.
		 * @param color the new clear color to use.
		 */
		virtual void setClearColor(const RGBAColorFloat& color) override
		{
		    mClearColor = color;
        }
		
		/**
		 * @return the currently used render target clear color.
		 */
		virtual const RGBAColorFloat& getClearColor() const override { return mClearColor; }

		/**
		 * Geometry winding order, defaults to clockwise. 
		 */
		virtual ECullWindingOrder getWindingOrder() const override				{ return ECullWindingOrder::Clockwise; }

		/**
		 * @return the render pass
		 */
		virtual VkRenderPass getRenderPass() const override						{ return mRenderPass; }

		/**
		 * @return the texture that holds the result of the render pass.
		 */
		RenderTexture2D& getColorTexture()										{ return *mColorTexture; }

		/**
		 * @return render target color format. This is the format of the linked in color texture.
		 */
		virtual VkFormat getColorFormat() const override						{ return mColorTexture->getFormat(); }

		/**
		 * @return render target depth format
		 */
		virtual VkFormat getDepthFormat() const override;

		/**
		 * @return used number of samples when rendering to the target.
		 */
		virtual VkSampleCountFlagBits getSampleCount() const override			{ return mRasterizationSamples; }
		
		/**
		 * @return if sample based shading is enabled when rendering to the target.
		 */
		virtual bool getSampleShadingEnabled() const override					{ return mSampleShading; }

		/**
		 * @return the full size of the current target texture in pixels.
		 */
		glm::ivec2 getFullBufferSize() const									{ return mColorTexture->getSize(); }

		/**
		 * @return the absolute size of a single view cell in pixels.
		 */
		glm::ivec2 getViewSize() const											{ return mViewSize; }

		/**
		 * @return the offset in pixels of the current view.
		 */
		glm::ivec2 getViewOffset() const;

		/**
		 * @return the quilt settings of the current render target.
		 */
		const QuiltSettings& getQuiltSettings() const							{ return mDevice->getQuiltSettings(); }

		/**
		 * Renders a quilt to nap::RenderTexture2D using the specified nap::QuiltRenderTarget. Use 'renderCallback' to
		 * group the rendering work of a single render pass. This pass is repeated a number of times to create the 
		 * resulting quilt texture. The perspective shift between views is handled automatically.
		 * Call this function as follows:
		 *
		 * ~~~~~{.cpp} 
		 *	if (mRenderService->beginHeadlessRecording())
		 *	{
		 *		quilt_target->render(quilt_camera, [render_service = mRenderService, comps = render_comps](QuiltRenderTarget& target, QuiltCameraComponentInstance& camera)
		 *		{
		 *			render_service->renderObjects(target, camera, comps);
		 *		});
		 *		mRenderService->endHeadlessRecording();
		 *	}
		 * ~~~~~
		 */
		void render(QuiltCameraComponentInstance& quiltCamera, std::function<void(nap::QuiltRenderTarget&, nap::QuiltCameraComponentInstance&)> renderCallback);

	public:
		bool									mSampleShading = true;								///< Property: 'SampleShading' Reduces texture aliasing when enabled, at higher computational cost.
		RGBAColorFloat 							mClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };			///< Property: 'ClearColor' color selection used for clearing the render target.
		ERasterizationSamples					mRequestedSamples = ERasterizationSamples::One;		///< Property: 'Samples' The number of samples used during Rasterization. For better results turn on 'SampleShading'.
		ResourcePtr<RenderTexture2D>			mColorTexture;										///< Property: 'ColorTexture' texture to render to.
		ResourcePtr<LookingGlassDevice>			mDevice;											///< Property: 'LookingGlassDevice' the looking glass device.

	private:
		RenderService*							mRenderService;
		VkFramebuffer							mFramebuffer = nullptr;
		VkRenderPass							mRenderPass = nullptr;
		VkSampleCountFlagBits					mRasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		ImageData								mDepthImage;
		ImageData								mColorImage;

		glm::ivec2								mViewSize;
		int										mCurrentView = 0;
		bool									mIsFirstPass = true;
	};
}
