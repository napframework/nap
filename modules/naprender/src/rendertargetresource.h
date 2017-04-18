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
		RTTI_ENABLE_DERIVED_FROM(Resource)
	public:

		virtual bool init(InitResult& initResult) override;

		/**
		* Sets color texture resource.
		*/
		void setColorTexture(MemoryTextureResource2D& colorTexture)			{ mColorTexture.setTarget(colorTexture); }

		/**
		* Sets depth texture resource
		*/
		void setDepthTexture(MemoryTextureResource2D& depthTexture)			{ mDepthTexture.setTarget(depthTexture); }

		/**
		* Returns color texture resource
		*/
		MemoryTextureResource2D& GetColorTexture()							{ return *mColorTexture.getTarget<MemoryTextureResource2D>();  }

		/**
		* Returns depth texture resource
		*/
		MemoryTextureResource2D& GetDepthTexture()							{ return *mDepthTexture.getTarget<MemoryTextureResource2D>();	}

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
		opengl::TextureRenderTarget2D mTextureRenderTarget;

		// Color texture to be used by the render target
		ObjectLinkAttribute mColorTexture = { this, "mColorTexture", RTTI_OF(MemoryTextureResource2D) };
		
		// Depth texture to be used by the render target
		ObjectLinkAttribute mDepthTexture = { this, "mDepthTexture", RTTI_OF(MemoryTextureResource2D) };
	};
}

RTTI_DECLARE(nap::TextureRenderTargetResource2D)
