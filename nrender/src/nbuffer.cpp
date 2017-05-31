#include "nbuffer.h"
#include <assert.h>

namespace opengl
{
	Buffer::~Buffer()
	{
		if (isAllocated())
		{
			glDeleteBuffers(1, &mId);
		}
	}

	// Initialize buffer
	void Buffer::init()
	{
		if (isAllocated())
		{
			printMessage(MessageType::WARNING, "buffer already allocated, generating new one");
			printMessage(MessageType::WARNING, "the previous buffer id will be invalidated");
			glDeleteBuffers(1, &mId);
		}
		glGenBuffers(1, &mId);
		glAssert();
	}


	// binds this vertex buffer on the GPU for subsequent GPU calls
	bool Buffer::bind() const
	{
		if (!isAllocated())
		{
			printMessage(MessageType::ERROR, "unable to bind vertex buffer: buffer is not allocated");
			return false;
		}
		glBindBuffer(getBufferType(), mId);
		return true;
	}


	// unbinds this vertex buffer on the GPU for subsequent GPU calls
	bool Buffer::unbind() const
	{
		if (!isAllocated())
		{
			printMessage(MessageType::ERROR, "unable to unbind vertex buffer: buffer is not allocated");
			return false;
		}
		glBindBuffer(getBufferType(), 0);
		return true;
	}

} // opengl