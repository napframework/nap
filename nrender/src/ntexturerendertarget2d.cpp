#include "ntexturerendertarget2d.h"
#include "ntexture2d.h"
#include "nglutils.h"

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
		glAssert();

		bind();

		// Attach color texture
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mColorTexture->getTargetType(), mColorTexture->getTextureId(), 0);
		glAssert();

		// Attach depth texture
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mDepthTexture->getTargetType(), mDepthTexture->getTextureId(), 0);
		glAssert();

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

		unbind();
		
		return valid;
	}


	const glm::ivec2 TextureRenderTarget2D::getSize() const
	{
		return glm::ivec2(mColorTexture->getSettings().mWidth, mColorTexture->getSettings().mHeight);
	}


	// Binds the render target so it can be used by subsequent render calls
	bool TextureRenderTarget2D::bind()
	{
		if (!isAllocated())
		{
			printMessage(MessageType::ERROR, "unable to bind render target, buffer is not allocated");
			return false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
		glViewport(0, 0, mColorTexture->getSettings().mWidth, mColorTexture->getSettings().mHeight);
		return true;
	}


	// Unbinds the render target from subsequent render calls
	bool TextureRenderTarget2D::unbind()
	{
		if (!isAllocated())
		{
			printMessage(MessageType::ERROR, "unable to unbind render target, buffer is not allocated");
			return false;
		}

		// release and forward
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Generate mipmaps if the texture filter is set up that way
		if (isMipMap(mColorTexture->getParameters().minFilter))
		{
			mColorTexture->generateMipMaps();
		}

		if (isMipMap(mDepthTexture->getParameters().minFilter))
		{
			mDepthTexture->generateMipMaps();
		}

		return true;
	}
}