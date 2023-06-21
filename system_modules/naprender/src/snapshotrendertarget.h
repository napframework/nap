/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "irendertarget.h"
#include "renderutils.h"
#include "snapshot.h"

// External Includes
#include <vulkan/vulkan_core.h>

namespace nap
{
	// Forward Declares
	class RenderService;
	class Core;

	/**
	* Special version of RenderTarget made to work exclusively in conjunction with Snapshot.
	* Renders to multiple VkFramebuffers in a sequence to reduce the memory consumption of an otherwise large Texture2D.
	* This makes a huge difference when MSAA samples > 1 is configured as fewer additional image resources may be allocated
	* when multiple samples are used.
	*/
	class NAPAPI SnapshotRenderTarget : public IRenderTarget
	{
	public:
		/**
		 * Every render target requires a reference to core.
		 * @param core link to a nap core instance
		 */
		SnapshotRenderTarget(Core& core);
		
		/**
		 * Destroys allocated render resources
		 */
		~SnapshotRenderTarget();

		/**
		 * Initializes the render target, including all the required resources.
		 * @param snapshot pointer to a snapshot object
		 * @param errorState contains the error if initialization failed.
		 * @return if initialization succeeded.
		 */
		bool init(Snapshot* snapshot, utility::ErrorState& errorState);

		/**
		 * Starts the render pass.
		 * Only start the render pass after a successful call to RenderService::beginHeadlessRecording().
		 */
		virtual void beginRendering() override;

		/**
		 * Ends the render pass. Always call this after beginRendering().
		*/
		virtual void endRendering() override;

		/**
		 * @return size in pixels of the render target.
		 */
		virtual const glm::ivec2 getBufferSize() const override;

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
		 * @return render target color format. This is the format of the linked in color texture.
		 */
		virtual VkFormat getColorFormat() const override;

		/**
		 * @return render target depth format
		 */
		virtual VkFormat getDepthFormat() const override;

		/**
		 * @return used number of samples when rendering to the target.
		 */
		virtual VkSampleCountFlagBits getSampleCount() const override;
		
		/**
		 * @return if sample based shading is enabled when rendering to the target.
		 */
		virtual bool getSampleShadingEnabled() const override;
		
		/**
		 * @param cellIndex change the index of the cell to setup for rendering
		 */
		void setCellIndex(uint32_t cellIndex)									{ mCellIndex = cellIndex; }


	private:
		RenderService*				mRenderService = nullptr;
		Snapshot*					mSnapshot = nullptr;

		std::vector<VkFramebuffer>	mFramebuffers;
		glm::u32vec2				mSize = { 0, 0 };

		bool						mSampleShading = true;
		RGBAColorFloat				mClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };

		VkSampleCountFlagBits		mRasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		VkFormat					mFormat = VK_FORMAT_R8G8B8A8_UNORM;
		VkRenderPass				mRenderPass = VK_NULL_HANDLE;
		ImageData					mDepthImage;
		ImageData					mColorImage;

		uint32_t					mCellIndex = 0;
	};
}
