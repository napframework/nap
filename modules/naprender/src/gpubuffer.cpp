// Local Includes
#include "gpubuffer.h"

// External Includes
#include "vulkan/vulkan.h"
#include <assert.h>
#include <string.h>

namespace nap
{
	GPUBuffer::GPUBuffer(VmaAllocator vmaAllocator) :
		mVmaAllocator(vmaAllocator)
	{
	}


	// Uploads the data block to the GPU
	void GPUBuffer::setDataInternal(VkPhysicalDevice physicalDevice, VkDevice device, void* data, int elementSize, size_t numVertices, size_t reservedNumVertices, VkBufferUsageFlagBits usage)
	{
		// TODO: handle 'usage': dynamic/state
		// TODO: handle reallocation of buffer
		// TODO: handle growing of buffer
		// TODO: destroy buffers

		int size = elementSize * numVertices;
		VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		allocInfo.flags = 0;

		VkResult result = vmaCreateBuffer(mVmaAllocator, &bufferInfo, &allocInfo, &mBuffer, &mAllocation, &mAllocationInfo);
		if (result != VK_SUCCESS)
		{
			// TODO: error handling
			return;
		}

		void* mapped_memory;
		if (vmaMapMemory(mVmaAllocator, mAllocation, &mapped_memory) != VK_SUCCESS)
			return;

		memcpy(mapped_memory, data, size);
		vmaUnmapMemory(mVmaAllocator, mAllocation);
	}
}