// Local Includes
#include "indexbuffer.h"

// External Includes
#include <assert.h>

namespace nap
{
	IndexBuffer::IndexBuffer(RenderService& renderService, EMeshDataUsage usage) :
		GPUBuffer(renderService, usage)
	{
	}

	// Uploads the data block to the GPU
	void IndexBuffer::setData(VkPhysicalDevice physicalDevice, VkDevice device, const std::vector<unsigned int>& indices)
	{
		mCount = indices.size();
		setDataInternal(physicalDevice, device, (void*)indices.data(), sizeof(int), indices.size(), indices.capacity(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	}
}