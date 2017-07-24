#pragma once

// Local Includes
#include "renderattributes.h"
#include "image.h"

// External Includes
#include <nap/objectptr.h>
#include <nframebuffer.h>

namespace nap
{
	/**
	 * Frame buffer specialization of the render target resource
	 * Wraps an opengl frame buffer (RGBA + DEPTH)
	 */
	class NAPAPI TextureRenderTarget2D : public rtti::RTTIObject
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
		void setColorTexture(MemoryTexture2D& colorTexture)			{ mColorTexture = &colorTexture; }

		/**
		* Sets depth texture resource
		*/
		void setDepthTexture(MemoryTexture2D& depthTexture)			{ mDepthTexture = &depthTexture; }

		/**
		* Returns color texture resource
		*/
		MemoryTexture2D& getColorTexture()							{ return *mColorTexture; }

		/**
		* Returns depth texture resource
		*/
		MemoryTexture2D& getDepthTexture()							{ return *mDepthTexture; }

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
		nap::ObjectPtr<MemoryTexture2D> mColorTexture = nullptr;
		
		// Depth texture to be used by the render target
		nap::ObjectPtr<MemoryTexture2D> mDepthTexture = nullptr;

		glm::vec4 mClearColor;
	};
}
