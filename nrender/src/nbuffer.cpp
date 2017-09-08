#include "nbuffer.h"
#include <assert.h>

namespace opengl
{
	Buffer::Buffer()
	{
		glGenBuffers(1, &mId);
		glAssert();
	}

	Buffer::~Buffer()
	{
		glDeleteBuffers(1, &mId);
	}


	// binds this vertex buffer on the GPU for subsequent GPU calls
	void Buffer::bind() const
	{
		glBindBuffer(getBufferType(), mId);
	}


	// unbinds this vertex buffer on the GPU for subsequent GPU calls
	void Buffer::unbind() const
	{
		glBindBuffer(getBufferType(), 0);
	}

} // opengl