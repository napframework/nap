#pragma once

// Local Includes
#include "texture2d.h"

// External Includes
#include <rtti/rttiobject.h>
#include <rtti/objectptr.h>
#include <glm/glm.hpp>
#include <ntexturerendertarget2d.h>

namespace opengl
{
	class RenderTarget;
}

namespace nap
{
	/**
	 * A resource that is used to render objects to an off screen surface (set of textures).
	 * This objects requires a link to a color and depth texture and internally manages an opengl render target.
	 * The result of the render step is stored in the linked textures.
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
		rtti::ObjectPtr<Texture2D>		mColorTexture = nullptr;	///< Property: 'mColorTexture' link to texture used to store the color values of a render step
		rtti::ObjectPtr<Texture2D>		mDepthTexture = nullptr;	///< Property: 'mDepthTexture' link to texture used to store the depth values of a render step
		glm::vec4						mClearColor;				///< Property: 'mClearColor' RGBA color used when clearing the render target
	};
}
