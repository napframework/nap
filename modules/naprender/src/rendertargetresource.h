#pragma once

// Local Includes
#include "renderattributes.h"
#include "imageresource.h"

// External Includes
#include <nap/resource.h>
#include <nframebuffer.h>

namespace nap
{
	/**
	 * Frame buffer specialization of the render target resource
	 * Wraps an opengl frame buffer (RGBA + DEPTH)
	 */
	class TextureRenderTargetResource2D : public Resource
	{
		RTTI_ENABLE(Resource)
	public:

		virtual bool init(InitResult& initResult) override;

		virtual void finish(Resource::EFinishMode mode) override;

		/**
		* Sets color texture resource.
		*/
		void setColorTexture(MemoryTextureResource2D& colorTexture)			{ mColorTexture = &colorTexture; }

		/**
		* Sets depth texture resource
		*/
		void setDepthTexture(MemoryTextureResource2D& depthTexture)			{ mDepthTexture = &depthTexture; }

		/**
		* Returns color texture resource
		*/
		MemoryTextureResource2D& GetColorTexture()							{ return *mColorTexture; }

		/**
		* Returns depth texture resource
		*/
		MemoryTextureResource2D& GetDepthTexture()							{ return *mDepthTexture; }

		/**
		* @return opengl base frame buffer object
		* Note that this implicitly initializes the frame buffer
		*/
		opengl::TextureRenderTarget2D& getTarget();

		/**
		* @return human readable display name
		*/
		virtual const std::string getDisplayName() const override;

	private:

		// Framebuffer to draw to
		opengl::TextureRenderTarget2D* mTextureRenderTarget = nullptr;
		opengl::TextureRenderTarget2D* mPrevTextureRenderTarget = nullptr;

	public:
		// Color texture to be used by the render target
		MemoryTextureResource2D* mColorTexture = nullptr;
		
		// Depth texture to be used by the render target
		MemoryTextureResource2D* mDepthTexture = nullptr;

		glm::vec4 mClearColor;
	};
}
