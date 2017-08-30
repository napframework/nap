// Local Includes
#include "nvertexbuffer.h"
#include "nglutils.h"

// External Includes
#include <GL/glew.h>
#include <assert.h>

namespace opengl
{
	VertexAttributeBuffer::VertexAttributeBuffer(GLenum type, unsigned int numComponents, unsigned int numVertices, GLenum usage) :
		mType(type),
		mNumComponents(numComponents),
		mNumVertices(numVertices),
		mUsage(usage)
	{
	}

	// Uploads the data block to the GPU
	void VertexAttributeBuffer::setData(void* data) const
	{
		bind();

		size_t size = getGLTypeSize(mType) * mNumComponents * mNumVertices;
		glBufferData(getBufferType(), size, data, mUsage);
		glAssert();

		unbind();
	}

} // opengl