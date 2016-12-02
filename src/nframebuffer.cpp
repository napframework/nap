#include "nframebuffer.h"
#include "nglutils.h"

// Delete framebuffer
opengl::FramebufferBase::~FramebufferBase()
{
	if (isAllocated())
		glDeleteFramebuffers(1, &mFbo);
}


// Creates the framebuffer on the GPU 
void opengl::FramebufferBase::init()
{
	if (isAllocated())
	{
		printMessage(MessageType::ERROR, "framebuffer already initialized");
		return;
	}

	// Generate framebuffer
	glGenFramebuffers(1, &mFbo);

	// Call derived implementation
	onInit();
}


// Checks if the framebuffer is allocated and valid for use
bool opengl::FramebufferBase::isValid()
{
	if (!isAllocated())
	{
		printMessage(MessageType::ERROR, "framebuffer is not allocated");
		return false;
	}

	bind();
	bool valid(false);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	switch (status)
	{	
	case GL_FRAMEBUFFER_COMPLETE:
		valid = true;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		printMessage(MessageType::ERROR, "framebuffer incomplete : attachment is NOT complete");
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		printMessage(MessageType::ERROR, "framebuffer incomplete: no image is attached to FBO");
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		printMessage(MessageType::ERROR, "framebuffer incomplete: draw buffer missing");
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		printMessage(MessageType::ERROR, "framebuffer incomplete: read buffer missing");
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		printMessage(MessageType::ERROR, "framebuffer incomplete: multi-sample missing");
		break;

	case GL_FRAMEBUFFER_UNSUPPORTED:
		printMessage(MessageType::ERROR, "framebuffer incomplete : unsupported by FBO implementation");
		break;

	default:
		printMessage(MessageType::ERROR, "framebuffer incomplete : unknown error");
		break;
	}

	unbind();
	return valid;
}


// Binds the framebuffer so it can be used by subsequent render calls
bool opengl::FramebufferBase::bind()
{
	if (!isAllocated())
	{
		printMessage(MessageType::ERROR, "unable to bind framebuffer, buffer is not allocated");
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
	return false;
}


// Unbinds the framebuffer from subsequent render calls
bool opengl::FramebufferBase::unbind()
{
	if (!isAllocated())
	{
		printMessage(MessageType::ERROR, "unable to unbind framebuffer, buffer is not allocated");
		return false;
	}

	// release and forward
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	onRelease();
	return true;
}


//////////////////////////////////////////////////////////////////////////
// Frame-buffer
//////////////////////////////////////////////////////////////////////////


// Updates settings associated with color and depth buffer
void opengl::FrameBuffer::allocate(unsigned int width, unsigned int height)
{
	if (!isAllocated())
	{
		printMessage(MessageType::WARNING, "unable to allocate frame buffer texture resources, framebuffer not initialized");
		return;
	}

	// Allocate memory
	Texture2DSettings color_settings;
	color_settings.width = static_cast<GLsizei>(width);
	color_settings.height = static_cast<GLsizei>(height);
	color_settings.internalFormat = GL_RGBA;
	color_settings.format = GL_RGBA;
	color_settings.type = GL_UNSIGNED_BYTE;

	mColorTexture.allocate(color_settings);

	// Allocate memory
	Texture2DSettings depth_settings;
	depth_settings.width = static_cast<GLsizei>(width);
	depth_settings.height = static_cast<GLsizei>(height);
	depth_settings.internalFormat = GL_DEPTH_COMPONENT;
	depth_settings.format = GL_DEPTH_COMPONENT;
	depth_settings.type = GL_FLOAT;

	mDepthTexture.allocate(depth_settings);
}


void opengl::FrameBuffer::onInit()
{
	// Create color texture
	mColorTexture.init();

	// Create depth texture
	mDepthTexture.init();

	// Bind
	FramebufferBase::bind();

	// Attach color texture
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mColorTexture.getTargetType(), mColorTexture.getTextureId(), 0);

	// Attach depth texture
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mDepthTexture.getTargetType(), mDepthTexture.getTextureId(), 0);

	// Unbind
	FramebufferBase::unbind();
}


// Generate mipmaps if the texture filter is set up that way
void opengl::FrameBuffer::onRelease()
{
	if (isMipMap(mColorTexture.getParameters().minFilter))
	{
		mColorTexture.generateMipMaps();
	}

	if (isMipMap(mDepthTexture.getParameters().minFilter))
	{
		mDepthTexture.generateMipMaps();
	}
}
