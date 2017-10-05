#include "nbackbufferrendertarget.h"
#include <GL/glew.h>


namespace opengl
{
	// Binds the backbuffer (resets OpenGL framebuffer)
	bool BackbufferRenderTarget::bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, mSize.x, mSize.y);
		return true;
	}
}
