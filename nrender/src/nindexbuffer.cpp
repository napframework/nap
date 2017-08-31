// Local Includes
#include "nindexbuffer.h"
#include "ngltypes.h"

// External Includes
#include <assert.h>

namespace opengl
{
	// Constructor
	IndexBuffer::IndexBuffer(GLenum usage) : mUsage(usage)
	{}


	// Upload data
	void IndexBuffer::setData(const std::vector<unsigned int>& indices)
	{
		mCount = indices.size();

		bind();

		size_t size = getGLTypeSize(GL_UNSIGNED_INT) * indices.size();
		glBufferData(getBufferType(), size, indices.data(), mUsage);
		glAssert();

		unbind();
	}
}