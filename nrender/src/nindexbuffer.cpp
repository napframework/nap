// Local Includes
#include "nindexbuffer.h"
#include "ngltypes.h"

// External Includes
#include <assert.h>

namespace opengl
{
	// Constructor
	IndexBuffer::IndexBuffer(GLenum usage) : mUsage(usage)
	{
	}
 
	// Uploads the data block to the GPU
	void IndexBuffer::setData(const std::vector<unsigned int>& indices)
	{
		bind();

		mCount = indices.size();
		size_t new_size = getGLTypeSize(GL_UNSIGNED_INT) * indices.size();
		if (new_size > mCurCapacity)
		{
			mCurCapacity = getGLTypeSize(GL_UNSIGNED_INT) * indices.capacity();
			glBufferData(getBufferType(), mCurCapacity, nullptr, mUsage);
		}

		glBufferSubData(getBufferType(), 0, new_size, indices.data());
		glAssert();

		unbind();
	}
}