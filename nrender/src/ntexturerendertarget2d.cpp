#include "ntexturerendertarget2d.h"
#include "ntexture2d.h"

// External Includes
#include <assert.h>
#include "utility/errorstate.h"

namespace opengl
{
	// Delete render target
	TextureRenderTarget2D::~TextureRenderTarget2D()
	{
		if (isAllocated())
			glDeleteFramebuffers(1, &mFbo);
	}


	// Creates the render target on the GPU 
	bool TextureRenderTarget2D::init(opengl::Texture2D& colorTexture, opengl::Texture2D& depthTexture, nap::utility::ErrorState& errorState)
	{
		assert (!isAllocated());
		mColorTexture = &colorTexture;
		mDepthTexture = &depthTexture;

		// Generate render target
		glGenFramebuffers(1, &mFbo);

		// Attach color texture
		if (!attachTexture(GL_COLOR_ATTACHMENT0, *mColorTexture, errorState))
			return false;

		// Attach depth texture
		if (!attachTexture(GL_DEPTH_ATTACHMENT, *mDepthTexture, errorState))
			return false;

		return true;
	}


	const glm::ivec2 TextureRenderTarget2D::getSize() const
	{
		return glm::ivec2();
		//return glm::ivec2(mColorTexture->getSettings().mWidth, mColorTexture->getSettings().mHeight);
	}


	bool TextureRenderTarget2D::switchColorTexture(opengl::Texture2D& colorTexture, nap::utility::ErrorState& error)
	{
		if (attachTexture(GL_COLOR_ATTACHMENT0, colorTexture, error))
		{
			mColorTexture = &colorTexture;
			return true;
		}
		return false;
	}


	bool TextureRenderTarget2D::switchDepthTexture(opengl::Texture2D& depthTexture, nap::utility::ErrorState& error)
	{
		if (attachTexture(GL_DEPTH_ATTACHMENT, depthTexture, error))
		{
			mDepthTexture = &depthTexture;
			return true;
		}
		return false;
	}


	bool TextureRenderTarget2D::validate(nap::utility::ErrorState& errorState)
	{
		// Check status
		bool valid = false;
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		switch (status)
		{
		case GL_FRAMEBUFFER_COMPLETE:
			valid = true;
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			errorState.fail("Render target incomplete: attachment is not complete");
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			errorState.fail("Render target incomplete: no image is attached to FBO");
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			errorState.fail("Render target incomplete: draw buffer missing");
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			errorState.fail("Render target incomplete: read buffer missing");
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			errorState.fail("Render target incomplete: multi-sample missing");
			break;

		case GL_FRAMEBUFFER_UNSUPPORTED:
			errorState.fail("Render target incomplete: unsupported by FBO implementation");
			break;

		default:
			errorState.fail("Render target incomplete: unknown error");
			break;
		}
		return valid;
	}


	bool TextureRenderTarget2D::attachTexture(GLenum attachment, const opengl::Texture2D& texture, nap::utility::ErrorState& error)
	{
		// Attach and ensure all went well
		if (!bindTarget())
		{
			error.fail("unable to bind render target");
			return false;
		}

		// Bind attachment
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, texture.getTargetType(), texture.getTextureId(), 0);

		// Unbind and validate
		bool success = validate(error);
		unbindTarget();
		return success;
	}


	bool TextureRenderTarget2D::unbindTarget()
	{
		if (!isAllocated())
		{
//			printMessage(EGLSLMessageType::Error, "unable to unbind render target, buffer is not allocated");
			return false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return true;
	}


	bool TextureRenderTarget2D::bindTarget()
	{
		if (!isAllocated())
		{
//			printMessage(EGLSLMessageType::Error, "unable to bind render target, buffer is not allocated");
			return false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
		return true;
	}

	/*
	// Binds the render target so it can be used by subsequent render calls
	bool TextureRenderTarget2D::bind()
	{
		// bind render target
		if (!bindTarget())
			return false;

		// Update viewport
		glViewport(0, 0, mColorTexture->getSettings().mWidth, mColorTexture->getSettings().mHeight);
		return true;
	}


	// Unbinds the render target from subsequent render calls
	bool TextureRenderTarget2D::unbind()
	{
		if (!unbindTarget())
			return false;

		// Generate mipmaps if the texture filter is set up that way
		if (isMipMap(mColorTexture->getParameters().minFilter))
			mColorTexture->generateMipMaps();

		if (isMipMap(mDepthTexture->getParameters().minFilter))
			mDepthTexture->generateMipMaps();

		return true;
	}*/
}