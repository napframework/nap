#pragma once

// External Includes
#include <nap/resource.h>
#include "irendertarget.h"
#include "rtti/objectptr.h"
#include "vulkan/vulkan_core.h"
#include "rendertexture2d.h"

namespace nap
{
	class RenderTexture2D;
	class RenderService;

	/**
	 * A resource that is used to render objects to an off screen surface (set of textures).
	 * This objects requires a link to a color and depth texture and internally manages an opengl render target.
	 * The result of the render step is stored in the linked textures.
	 */
	class NAPAPI RenderTarget : public Resource, public IRenderTarget
	{
		RTTI_ENABLE(Resource)
	public:
		RenderTarget(Core& core);
		~RenderTarget();

		/**
		* Creates internal OpengL render target, bound to color and depth textures.
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		virtual void beginRendering() override;
		virtual void endRendering() override;

		virtual const glm::ivec2 getBufferSize() const;

		virtual void setClearColor(const glm::vec4& color) override { mClearColor = color; }
		virtual const glm::vec4& getClearColor() const override { return mClearColor; }

		virtual ECullWindingOrder getWindingOrder() const override { return ECullWindingOrder::Clockwise; }

		virtual VkRenderPass getRenderPass() const { return mRenderPass; }

		RenderTexture2D& getColorTexture();

		virtual VkFormat getColorFormat() const override;
		virtual VkFormat getDepthFormat() const override;
		virtual VkSampleCountFlagBits getSampleCount() const override;
		virtual bool getSampleShadingEnabled() const override;

	public:
		bool								mSampleShading = true;								///< Property: 'SampleShading' Reduces texture aliasing when enabled, at higher computational cost.
		glm::vec4							mClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };			///< Property: 'ClearColor' color selection used for clearing the render target
		ERasterizationSamples				mRequestedSamples = ERasterizationSamples::Four;	///< Property: 'Samples' Controls the number of samples used during Rasterization. For even better results turn on 'SampleShading'.
		rtti::ObjectPtr<RenderTexture2D>	mColorTexture;										///< Property: 'ColorTexture' texture to render to, format needs to be: 'Backbuffer'

	private:
		RenderService*			mRenderService;
		VkFramebuffer			mFramebuffer = nullptr;
		VkRenderPass			mRenderPass = nullptr;
		VkSampleCountFlagBits	mRasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		ImageData				mDepthImage;
		ImageData				mColorImage;
	};
}
