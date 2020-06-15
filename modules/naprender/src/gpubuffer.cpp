// Local Includes
#include "gpubuffer.h"
#include "rtti/typeinfo.h"

// External Includes
#include "vulkan/vulkan.h"
#include <assert.h>
#include <string.h>
#include "renderservice.h"
#include "nap/logger.h"

RTTI_BEGIN_ENUM(nap::EMeshDataUsage)
	RTTI_ENUM_VALUE(nap::EMeshDataUsage::Static,		"Static"),
	RTTI_ENUM_VALUE(nap::EMeshDataUsage::DynamicWrite,	"DynamicWrite")
RTTI_END_ENUM

namespace nap
{
	GPUBuffer::GPUBuffer(RenderService& renderService, EMeshDataUsage usage) :
		mRenderService(&renderService),
		mUsage(usage)
	{
		mBuffers.resize(mUsage == EMeshDataUsage::Static ? 1 : renderService.getMaxFramesInFlight() + 1);
	}


	GPUBuffer::~GPUBuffer()
	{
		mRenderService->queueVulkanObjectDestructor([buffers = mBuffers](RenderService& renderService)
		{
			for (const BufferData& buffer : buffers)
				if (buffer.mAllocation != VK_NULL_HANDLE)
					vmaDestroyBuffer(renderService.getVulkanAllocator(), buffer.mBuffer, buffer.mAllocation);
		});
	}

	
	void GPUBuffer::setDataInternal(VkPhysicalDevice physicalDevice, VkDevice device, void* data, int elementSize, size_t numVertices, size_t reservedNumVertices, VkBufferUsageFlagBits usage)
	{
		if (numVertices == 0)
			return;

		// For each update of data, we cycle through the buffers. This has the effect that if you only ever need a single buffer (static data), you 
		// will use only one buffer.
		mCurrentBufferIndex = (mCurrentBufferIndex + 1) % mBuffers.size();

		uint32_t required_size_bytes = elementSize * reservedNumVertices;

		VmaAllocator allocator = mRenderService->getVulkanAllocator();

		// If we didn't allocate a buffer yet, or if the buffer has grown, we allocate it. The buffer size depends on reservedNumVertices.
		BufferData& buffer_data = mBuffers[mCurrentBufferIndex];
		if (buffer_data.mBuffer == VK_NULL_HANDLE || required_size_bytes > buffer_data.mAllocationInfo.size)
		{
			if (buffer_data.mBuffer != VK_NULL_HANDLE)
				vmaDestroyBuffer(allocator, buffer_data.mBuffer, buffer_data.mAllocation);

			VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			bufferInfo.size = required_size_bytes;
			bufferInfo.usage = usage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			// We use CPU_TO_GPU to be able to update on CPU easily. This is mostly convenient for dynamic geometry and suboptimal
			// for static geometry. To solve this, we'd need to have special handling for static geometry where we use a GPU only buffer
			// and use a staging buffer to upload from CPU to GPU.
			VmaAllocationCreateInfo allocInfo = {};
			allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			allocInfo.flags = 0;

			VkResult result = vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer_data.mBuffer, &buffer_data.mAllocation, &buffer_data.mAllocationInfo);
			assert(result == VK_SUCCESS);
		}

		uint32_t used_size_bytes = elementSize * numVertices;

		void* mapped_memory;
		VkResult result = vmaMapMemory(allocator, buffer_data.mAllocation, &mapped_memory);
		assert(result == VK_SUCCESS);

		memcpy(mapped_memory, data, used_size_bytes);
		vmaUnmapMemory(allocator, buffer_data.mAllocation);

		bufferChanged();
	}
}