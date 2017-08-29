// Local Includes
#include "nvertexbuffer.h"
#include "nglutils.h"

// External Includes
#include <GL/glew.h>
#include <assert.h>

namespace opengl
{
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