#pragma once

#include <GL/glew.h>
#include "nrendertarget.h"

namespace nap
{
	namespace utility
	{
		class ErrorState;
	}
}

namespace opengl
{
	class Texture2D;

	/**
	* TextureRenderTarget2D
	*
	* Used for rendering to off screen buffer locations, without disrupting the main screen.
	* Every TextureRenderTarget has a color and depth texture attached to it.
	* When binding the TextureRenderTarget both color and depth are rendered to.
	* After rendering both textures can be used as a default texture in a shader or anywhere else in the GL pipeline
	*
	* For now only 1 color attachment is supported
	*/
	class TextureRenderTarget2D : public RenderTarget
	{
	public:
		// Constructor
		TextureRenderTarget2D() = default;
		~TextureRenderTarget2D();

		/**
		* Creates the render target on the GPU and binds the color and depth texture to their respective attachments.
		* This needs to be called first after construction otherwise subsequent calls will fail
		*/
		bool init(opengl::Texture2D& colorTexture, opengl::Texture2D& depthTexture, nap::utility::ErrorState& errorState);

		/**
		* Binds the render target so it can be used by subsequent render calls.
		* @return if the FBO was successfully bound or not
		*/
		virtual bool bind() override;

		/**
		 * Unbinds the render target.
		 * Note that if the texture supports lods mip-maps are generated automatically.
		 * @return if unbinding the target succeeded.
		 */
		virtual bool unbind() override;

		/**
		* @return the texture associated with the color channel of this render target
		*/
		opengl::Texture2D& getColorTexture()			{ assert(mColorTexture != nullptr);  return *mColorTexture; }

		/**
		* @return the texture associated with the depth channel of this render target
		*/
		opengl::Texture2D& getDepthTexture()			{ assert(mDepthTexture != nullptr);  return *mDepthTexture; }

		/**
		 * @return Size of the render-target, in texels.
		 */
		virtual const glm::ivec2 getSize() const override;

		/**
		 * Sets the color texture to use, this call will fail if the target hasn't been initialized yet.
		 * Note that the target texture needs to have the same size as the current color texture.
		 * @param colorTexture texture to use
		 * @param error contains the error if the operation fails
		 * @return if switching the color texture succeeded.
		 */
		bool switchColorTexture(opengl::Texture2D& colorTexture, nap::utility::ErrorState& error);

		/**
		 * Sets the depth texture to use, this call will fail if the target hasn't been initialized yet.
		 * Note that the target texture needs to have the same size as the current depth texture.
		 * @param depthTexture texture to use
		 * @param error contains the error if the operation fails.
		 * @return if switching the depth texture succeeded.
		 */
		bool switchDepthTexture(opengl::Texture2D& depthTexture, nap::utility::ErrorState& error);

	private:
		/**
		* @return if the render target is allocated (created) on the GPU
		*/
		bool isAllocated() const { return mFbo != 0; }

	private:
		GLuint						mFbo = 0;					// FBO GPU id
		opengl::Texture2D*			mColorTexture = nullptr;	// Color texture
		opengl::Texture2D*			mDepthTexture = nullptr;	// Depth texture

		/**
		 * Attaches a texture to the framebuffer.
		 * @param attachment attachment type, ie: GL_COLOR_ATTACHMENT0 etc.
		 * @param texture opengl texture to associate with attachment
		 * @param error contains the error if attaching the texture fails
		 */
		bool attachTexture(GLenum attachment, const opengl::Texture2D& texture, nap::utility::ErrorState& error);

		/**
		 * Unbinds the render target
		 * @return if operation succeeded.
		 */
		bool unbindTarget();

		/**
		 * Binds the render target
		 * @return if operation succeeded.
		 */
		bool bindTarget();

		/**
		 * Validates the framebuffer after a frame-buffer operation, such as setting a texture target.
		 * @param error contains the error if validation fails.
		 * @return if the buffer is valid or not after an operation.
		 */
		bool validate(nap::utility::ErrorState& error);
	};

} // opengl
