#pragma once

// Local Includes
#include "ntexture.h"

// External Includes
#include <GL/glew.h>

namespace opengl
{
	/**
	 * Used for rendering to off screen buffer locations, without disrupting the main screen.
	 * This is the base framebuffer that is incomplete on construction and initialization
	 * Derived classes should implement specific frame buffer functionality such as rendering 
	 * color, z-depth, stencil etc to textures or buffers. 
	 */
	class FramebufferBase
	{
	public:
		// Constructor
		FramebufferBase() = default;

		// Destructor
		virtual ~FramebufferBase();

		// Don't support copy
		FramebufferBase(const FramebufferBase&) = delete;
		FramebufferBase& operator=(const FramebufferBase&) = delete;

		/**
		* Creates the framebuffer on the GPU and initializes
		* This needs to be called first after construction otherwise subsequent calls will fail
		*/
		void init();

		/**
		* @return if the framebuffer is allocated (created) on the GPU
		*/
		bool isAllocated() const					{ return mFbo != 0; }

		/**
		* @return if the framebuffer is allocated and valid for use
		* ie, if attachments are valid and complete
		*/
		bool isValid();

		/**
		* Binds the framebuffer so it can be used by subsequent render calls
		* @return if the FBO was successfully bound or not
		*/
		bool bind();

		/**
		* Unbinds the framebuffer, future framebuffer calls won't have any effect on this framebuffer
		* @return if the FBO was successfully unbound
		*/
		bool unbind();

		/**
		 * @return framebuffer GPU identifier
		 */
		GLuint getFramebufferId() const				{ return mFbo; }

	protected:
		GLuint		mFbo = 0;			// FBO GPU id

		/**
		* Should be implemented by derived frame buffer objects
		* Often used to initialize frame specific textures or buffers
		*/
		virtual void onInit() = 0;

		/**
		 * Called when the buffer is un-bound
		 * The actual framebuffer has already been released, use this override
		 * to clean up or post process changes afterwards
		 */
		virtual void onRelease()						{ }
	};


	/**
	 * Framebuffer
	 *
	 * Used for rendering to off screen buffer locations, without disrupting the main screen.
	 * Every framebuffer has a color and depth texture attached to it. 
	 * When binding the framebuffer both color and depth are rendered to. 
	 * After rendering both textures can be used as a default texture in a shader or anywhere else in the GL pipeline
	 * 
	 * By default the color texture is of type: RGBA and has 8 bits of depth per component (unsigned byte)
	 * The depth texture is of type: GL_DEPTH_COMPONENT and has 32 bits of depth (float).
	 * You can change the format and type by querying the texture and changing it's settings
	 *
	 * For now only 1 color attachment is supported 
	 */
	class FrameBuffer : public FramebufferBase
	{
	public:
		// Constructor
		FrameBuffer() = default;

		/**
		 * allocates the memory on the GPU for the textures associated with the framebuffer
		 * Without allocating any memory for the textures on the GPU the framebuffer is invalid
		 */
		void allocate(unsigned int width, unsigned int height);

		/**
		 * @return the texture associated with the color channel of this framebuffer
		 */
		Texture2D& getColorTexture() 					{ return mColorTexture; }
		
		/**
		 * @return the texture associated with the depth channel of this framebuffer
		 */
		Texture2D& getDepthTexture() 					{ return mDepthTexture; }

	protected:
		/**
		 * Creates the textures to be used in conjunction with the framebuffer
		 * This needs to be called first after construction otherwise subsequent calls will fail
		 */
		virtual void onInit() override;

		/**
		 * Generates mip maps
		 */
		virtual void onRelease() override;

	private:
		Texture2D	mColorTexture;				// used for rendering color
		Texture2D	mDepthTexture;				// used for rendering depth
	};

} // opengl
