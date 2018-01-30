#pragma once

// Local Includes
#include "texture2d.h"

// External Includes
#include <rtti/rttiobject.h>
#include <nap/objectptr.h>
#include <glm/glm.hpp>
#include <ntexturerendertarget2d.h>

namespace opengl
{
	class RenderTarget;
}

namespace nap
{
	/**
	 * Frame buffer specialization of the render target resource
	 * Wraps an opengl frame buffer (RGBA + DEPTH)
	 */
	class NAPAPI RenderTarget : public rtti::RTTIObject
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
		void setColorTexture(Texture2D& colorTexture)				{ mColorTexture = &colorTexture; }

		/**
		* Sets depth texture resource
		*/
		void setDepthTexture(Texture2D& depthTexture)				{ mDepthTexture = &depthTexture; }

		/**
		* Returns color texture resource
		*/
		Texture2D& getColorTexture()								{ return *mColorTexture; }

		/**
		* Returns depth texture resource
		*/
		Texture2D& getDepthTexture()								{ return *mDepthTexture; }

		/**
		* @return opengl base frame buffer object
		* Note that this implicitly initializes the frame buffer
		*/
		opengl::TextureRenderTarget2D& getTarget();

	private:

		// Frame-buffer to draw to
		std::unique_ptr<opengl::TextureRenderTarget2D> mTextureRenderTarget = nullptr;

	public:
		nap::ObjectPtr<Texture2D>	mColorTexture = nullptr;	// Color texture to be used by the render target
		nap::ObjectPtr<Texture2D>	mDepthTexture = nullptr;	// Depth texture to be used by the render target
		glm::vec4						mClearColor;				// Color used when clearing the render target
	};
}
