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
			{
				// The very first time we set data, we allocate a buffer to the exact size that was requested.
				mCurCapacity = mCurSize;
			}
			else
			{
				// In case there was already data but the size does not match anymore, we find the nearest 
				// 'next' power-of-two to grow to. This means that we are roughly growing by a factor of two 
				// if the amount of vertices grows gradually.
				double exponent = std::ceil(std::log2(mCurSize));
				mCurCapacity = std::exp2(exponent);
			}
			assert(mCurCapacity >= mCurSize);
			glBufferData(getBufferType(), mCurCapacity, nullptr, mUsage);
		}

		glBufferSubData(getBufferType(), 0, mCurSize, data);
		glAssert();

		unbind();
	}

} // opengl