#pragma once

#include <GL/glew.h>
#include "nrendertarget.h"

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
		* Creates the render target on the GPU and initializes
		* This needs to be called first after construction otherwise subsequent calls will fail
		*/
		void init(opengl::Texture2D& colorTexture, opengl::Texture2D& depthTexture, const glm::vec4& clearColor);

		/**
		* @return if the render target is allocated (created) on the GPU
		*/
		bool isAllocated() const { return mFbo != 0; }

		/**
		* @return if the render target is allocated and valid for use
		* ie, if attachments are valid and complete
		*/
		virtual bool isValid() override;

		/**
		* Binds the render target so it can be used by subsequent render calls
		* @return if the FBO was successfully bound or not
		*/
		virtual bool bind() override;

		/**
		* Unbinds the render target
		*/
		virtual bool unbind() override;

		/**
		* @return the texture associated with the color channel of this render target
		*/
		opengl::Texture2D& getColorTexture() { assert(mColorTexture != nullptr);  return *mColorTexture; }

		/**
		* @return the texture associated with the depth channel of this render target
		*/
		opengl::Texture2D& getDepthTexture() { assert(mDepthTexture != nullptr);  return *mDepthTexture; }

		/**
		* @return Size of the rendertarget, in texels.
		*/
		virtual const glm::ivec2 getSize() const override;

	private:
		/**
		* Generates the OpenGL FBO object, attaches color and depth textures.
		*/
		void allocate(opengl::Texture2D& colorTexture, opengl::Texture2D& depthTexture);

	private:
		GLuint						mFbo = 0;					// FBO GPU id
		opengl::Texture2D*			mColorTexture = nullptr;	// Color texture
		opengl::Texture2D*			mDepthTexture = nullptr;	// Depth texture
	};

} // opengl
