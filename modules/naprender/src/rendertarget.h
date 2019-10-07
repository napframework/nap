#pragma once

// Local Includes
#include "texture2d.h"

// External Includes
#include <nap/resourceptr.h>
#include <glm/glm.hpp>
#include <ntexturerendertarget2d.h>
#include <nap/resource.h>

namespace opengl
{
	class RenderTarget;
}

namespace nap
{
	enum class ERenderTargetFormat
	{
//		Backbuffer,		///< The current native format of the color backbuffer
		RGBA8,			///< RGBA8 4 components, 8 bytes per component
		RGB8,			///< RGB8 3 components, 8 bytes per component
		R8,				///< R8	1 components, 8 bytes per component
		Depth			///< Depth Texture used for binding to depth buffer
	};

	/**
	 * A resource that is used to render objects to an off screen surface (set of textures).
	 * This objects requires a link to a color and depth texture and internally manages an opengl render target.
	 * The result of the render step is stored in the linked textures.
	 */
	class NAPAPI RenderTarget : public Resource
	{
		RTTI_ENABLE(Resource)
	public:

		/**
		* Creates internal OpengL render target, bound to color and depth textures.
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Sets color texture to use by the render-target.
		 * Note that if the operation fails the previous texture remains active and bound.
		 * Target size needs to match current color texture size.
		 * @param colorTexture color texture to render to.
		 * @param error contains the error if setting the texture fails
		 * @return if setting the texture succeeded.
		 */
		bool switchColorTexture(Texture2D& colorTexture, utility::ErrorState& error);

		/**
		 * Sets depth texture to use by the render-target.
		 * Note that if the operation fails the previous texture remains active and bound.
		 * Target size needs to match current depth texture size.
		 * @param depthTexture depth texture to render to.
		 * @param error contains the error if setting the texture fails
		 * @return if setting the texture succeeded.
		 */
		bool switchDepthTexture(Texture2D& depthTexture, utility::ErrorState& error);

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
		ResourcePtr<Texture2D>			mColorTexture = nullptr;	///< Property: 'mColorTexture' link to texture used to store the color values of a render step
		ResourcePtr<Texture2D>			mDepthTexture = nullptr;	///< Property: 'mDepthTexture' link to texture used to store the depth values of a render step
		glm::vec4						mClearColor;				///< Property: 'mClearColor' RGBA color used when clearing the render target
	};
}
