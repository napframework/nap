// Local Includes
#include "nvertexbuffer.h"
#include "nglutils.h"

// External Includes
#include <GL/glew.h>
#include <assert.h>

namespace opengl
{
	// Uploads the data block to the GPU
	void VertexAttributeBuffer::setData(void* data)
	{
		if (!bind())
			return;

		size_t size = getGLTypeSize(mType) * mNumComponents * mNumVertices;
		glBufferData(getBufferType(), size, data, mUsage);
		glAssert();

		// Unbind buffer after setting data
		unbind();
	}

} // opengl