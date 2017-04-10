#pragma once

// Local Includes
#include "ntexture.h"

// External Includes
#include <GL/glew.h>
#include <glm/glm.hpp>

namespace opengl
{
	/**
	* Flags used to choose what part of render target to clear
	*/
	enum class EClearFlags : uint8_t
	{
		Color = 1,
		Depth = 2,
		Stencil = 4
	};

	inline EClearFlags operator&(EClearFlags a, EClearFlags b)
	{
		return static_cast<EClearFlags>(static_cast<int>(a) & static_cast<int>(b));
	}
	inline EClearFlags operator|(EClearFlags a, EClearFlags b)
	{
		return static_cast<EClearFlags>(static_cast<int>(a) | static_cast<int>(b));
	}


	/**
	 * Base class for TextureRenderTargets and BackBufferRenderTargets.
	 */
	class RenderTarget
	{
	public:
		// Constructor
		RenderTarget() = default;

		// Destructor
		virtual ~RenderTarget() = default;

		// Don't support copy
		RenderTarget(const RenderTarget&) = delete;
		RenderTarget& operator=(const RenderTarget&) = delete;

		/**
		* @return if the renderTarget is allocated and valid for use
		*/
		virtual bool isValid() = 0;

		/**
		* Binds the render target so it can be used by subsequent render calls
		*/
		virtual bool bind() = 0;

		/**
		* Unbinds the render target. TODO: decide how to proceeed with unbinding render targets.
		*/
		virtual bool unbind() = 0;

		/**
		* Clears color, depth and stencil depending on flags. Uses ClearColor.
		*/
		virtual void clear(EClearFlags flags);

		/**
		* Sets the clear color to be used by clear.
		*/
		void setClearColor(const glm::vec4 color)		{ mClearColor = color; }

	private:
		glm::vec4 mClearColor;			// Clear color, used for clearing the color buffer
	};

	/**
	 * TextureRenderTarget
	 *
	 * Used for rendering to off screen buffer locations, without disrupting the main screen.
	 * Every TextureRenderTarget has a color and depth texture attached to it. 
	 * When binding the TextureRenderTarget both color and depth are rendered to. 
	 * After rendering both textures can be used as a default texture in a shader or anywhere else in the GL pipeline
	 * 
	 * For now only 1 color attachment is supported 
	 */
	class TextureRenderTarget : public RenderTarget
	{
	public:
		// Constructor
		TextureRenderTarget() = default;
		~TextureRenderTarget();

		/**
		* Creates the render target on the GPU and initializes
		* This needs to be called first after construction otherwise subsequent calls will fail
		*/
		void init(opengl::Texture2D& colorTexture, opengl::Texture2D& depthTexture);

		/**
		* @return if the render target is allocated (created) on the GPU
		*/
		bool isAllocated() const { return mFbo != 0; }

		/**
		 * Generates the OpenGL fbo object, attaches color and depth textures.
		 */
		void allocate(opengl::Texture2D& colorTexture, opengl::Texture2D& depthTexture);

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
		opengl::Texture2D& getColorTexture()					{ assert(mColorTexture != nullptr);  return *mColorTexture; }
		
		/**
		 * @return the texture associated with the depth channel of this render target
		 */
		opengl::Texture2D& getDepthTexture() 					{ assert(mDepthTexture != nullptr);  return *mDepthTexture; }

	private:
		GLuint						mFbo = 0;					// FBO GPU id
		opengl::Texture2D*			mColorTexture = nullptr;	// Color texture
		opengl::Texture2D*			mDepthTexture = nullptr;	// Depth texture
	};
	
	/**
	* Render target that represents a backbuffer.
	*/
	class BackbufferRenderTarget : public RenderTarget
	{
	public:
		/**
		Backbuffer is always valid. Returns true.
		*/
		virtual bool isValid() override { return true; }

		/**
		* Binds the framebuffer so it can be used by subsequent render calls
		*/
		virtual bool bind() override;

		/**
		* NOP. TODO: decide how to proceed with unbinding render targets
		*/
		virtual bool unbind() override { return true; }

	};

} // opengl
