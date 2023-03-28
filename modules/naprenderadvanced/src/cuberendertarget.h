/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "irendertarget.h"
#include "rendertexture2d.h"

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <vulkan/vulkan_core.h>

namespace nap
{
	// Forward Declares
	class TextureCube;
	class RenderTexture2D;
	class RenderService;
	class PerspCameraComponentInstance;

	//////////////////////////////////////////////////////////////////////////
	// Cube RenderTarget
	//////////////////////////////////////////////////////////////////////////

	/**
	 * CubeRenderTarget
	 *
	 * A specialized version of nap::RenderTarget that renders a quilt texture, comprising a number of views of one or
	 * multiple objects in a single nap::RenderTexture2D. The layout of the quilt is determined by the nap::QuiltSettings
	 * acquired from the nap::LookingGlassDevice resource in the "Device" property. 
	 *
	 * When rendering, the perspective shift between views is handled automatically by nap::CubeRenderTarget and 
	 * specified nap::QuiltCameraComponentInstance when using the function CubeRenderTarget::render(). This is 
	 * necessary as the rendering of multiple views requires the setup of a renderpass for each view, as opposed to 
	 * regular use of nap::RenderTarget or nap::RenderWindow. Refer to the following example:
	 *
	 * ~~~~~{.cpp} 
	 *	if (mRenderService->beginHeadlessRecording())
	 *	{
	 *		quilt_target->render(quilt_camera, [render_service = mRenderService, comps = render_comps](CubeRenderTarget& target, QuiltCameraComponentInstance& camera)
	 *		{
	 *			render_service->renderObjects(target, camera, comps);
	 *		});
	 *		mRenderService->endHeadlessRecording();
	 *	}
	 * ~~~~~
	 */
	class NAPAPI CubeRenderTarget : public Resource, public IRenderTarget
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		 * Every render target requires a reference to core.
		 * @param core link to a nap core instance
		 */
		CubeRenderTarget(Core& core);
		
		/**
		 * Destroys allocated render resources
		 */
		~CubeRenderTarget();

		/**
		 * Initializes the render target, including all the required resources.
		 * @param errorState contains the error if initialization failed.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Starts the render pass. Called by CubeRenderTarget::render().
		 */
		virtual void beginRendering() override;

		/**
		 * Ends the render pass. Called by CubeRenderTarget::render().
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
		virtual void setClearColor(const RGBAColorFloat& color) override		{ mClearColor = color; }
		
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
		 * @return the texture that holds the result of the render pass.
		 */
		//RenderTexture2D& getColorTexture()									{ return *mColorTexture; }

		/**
		 * @return render target color format.
		 */
		virtual VkFormat getColorFormat() const override						{ return mVulkanColorFormat; }

		/**
		 * @return render target depth format
		 */
		virtual VkFormat getDepthFormat() const override						{ return mVulkanDepthFormat; }

		/**
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
		 * Renders a quilt to nap::RenderTexture2D using the specified nap::CubeRenderTarget. Use 'renderCallback' to
		 * group the rendering work of a single render pass. This pass is repeated a number of times to create the 
		 * resulting quilt texture. The perspective shift between views is handled automatically.
		 * Call this function as follows:
		 *
		 * ~~~~~{.cpp} 
		 *	if (mRenderService->beginHeadlessRecording())
		 *	{
		 *		quilt_target->render(quilt_camera, [render_service = mRenderService, comps = render_comps](CubeRenderTarget& target, QuiltCameraComponentInstance& camera)
		 *		{
		 *			render_service->renderObjects(target, camera, comps);
		 *		});
		 *		mRenderService->endHeadlessRecording();
		 *	}
		 * ~~~~~
		 */
		void render(PerspCameraComponentInstance& camera, std::function<void(CubeRenderTarget&, const glm::mat4& projection, const glm::mat4& view)> renderCallback);

		/**
		 * 
		 */
		void render(const glm::vec3& camPosition, const glm::mat4& projectionMatrix, std::function<void(CubeRenderTarget&, const glm::mat4& projection, const glm::mat4& view)> renderCallback);

		/**
		 * 
		 */
		void setLayerIndex(uint index);

		/**
		 * 
		 */
		uint getLayerIndex() const												{ return mLayerIndex; }

		/**
		 * 
		 */
		static const std::vector<glm::mat4>& getCubeMapViewMatrices();

		/**
		 * 
		 */
		static const std::vector<glm::mat4>& getCubeMapInverseViewMatrices();


		bool									mSampleShading = true;								///< Property: 'SampleShading' Reduces texture aliasing when enabled, at higher computational cost.
		RGBAColorFloat							mClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };			///< Property: 'ClearColor' color selection used for clearing the render target.

		RenderTexture2D::EFormat				mColorFormat = RenderTexture2D::EFormat::RGBA8;
		DepthRenderTexture2D::EDepthFormat		mDepthFormat = DepthRenderTexture2D::EDepthFormat::D16;

		ResourcePtr<TextureCube>				mCubeTexture;										///< Property: 'CubeTexture' texture to render to.

	private:
		RenderService*							mRenderService;
		VkRenderPass							mRenderPass = VK_NULL_HANDLE;

		VkFormat								mVulkanColorFormat = VK_FORMAT_UNDEFINED;
		VkFormat								mVulkanDepthFormat = VK_FORMAT_UNDEFINED;

		ImageData								mDepthImage{ TextureCube::LAYER_COUNT };
		ImageData								mColorImage{ TextureCube::LAYER_COUNT };

		std::array<VkFramebuffer, TextureCube::LAYER_COUNT>	mFramebuffers;

		glm::ivec2								mSize;
		uint									mLayerIndex = 0U;
		bool									mIsFirstPass = true;
	};
}
