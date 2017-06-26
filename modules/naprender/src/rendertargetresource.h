#pragma once

// Local Includes
#include "renderattributes.h"
#include "imageresource.h"

// External Includes
#include <nap/resource.h>
#include <nap/objectptr.h>
#include <nframebuffer.h>

namespace nap
{
	/**
	 * Frame buffer specialization of the render target resource
	 * Wraps an opengl frame buffer (RGBA + DEPTH)
	 */
	class TextureRenderTargetResource2D : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:

		/**
		* Creates internal OpengL render target, bound to color and depth textures.
		*/
		virtual bool init(utility::ErrorState& errorState) override;

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

	private:

		// Framebuffer to draw to
		opengl::TextureRenderTarget2D* mTextureRenderTarget = nullptr;

	public:
		// Color texture to be used by the render target
		nap::ObjectPtr<MemoryTextureResource2D> mColorTexture = nullptr;
		
		// Depth texture to be used by the render target
		nap::ObjectPtr<MemoryTextureResource2D> mDepthTexture = nullptr;

		glm::vec4 mClearColor;
	};
}
