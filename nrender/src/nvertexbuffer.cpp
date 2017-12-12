// Local Includes
#include "nvertexbuffer.h"
#include "nglutils.h"

// External Includes
#include <GL/glew.h>
#include <assert.h>

namespace opengl
{
	VertexAttributeBuffer::VertexAttributeBuffer(GLenum type, unsigned int numComponents, GLenum usage) :
		mType(type),
		mNumComponents(numComponents),
		mCurCapacity(0),
		mCurSize(0),
		mUsage(usage)
	{
	}

	// Uploads the data block to the GPU
	void VertexAttributeBuffer::setData(void* data, unsigned int numVertices)
	{
		bind();

		mCurSize = getGLTypeSize(mType) * mNumComponents * numVertices;
		if (mCurSize > mCurCapacity)
		{
			if (mCurCapacity == 0)
				mCurCapacity = 64;

			while (mCurCapacity < mCurSize)
				mCurCapacity *= 2;

			glBufferData(getBufferType(), mCurCapacity, nullptr, mUsage);
		}

		glBufferSubData(getBufferType(), 0, mCurSize, data);
		glAssert();

		unbind();
	}

} // opengl