// Local Includes
#include "nindexbuffer.h"

// External Includes
#include <assert.h>

namespace opengl
{
	// Uploads the data block to the GPU
	void IndexBuffer::setData(VkPhysicalDevice physicalDevice, VkDevice device, const std::vector<unsigned int>& indices)
	{
		mCount = indices.size();
		setDataInternal(physicalDevice, device, (void*)indices.data(), sizeof(int), indices.size(), indices.capacity(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

		/*
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
		*/
	}
}