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
		// Scale buffers based on number of frames in flight when not static.
		mRenderBuffers.resize(mUsage == EMeshDataUsage::Static ? 1 : 
			renderService.getMaxFramesInFlight() + 1);
	}


	VkBuffer GPUBuffer::getBuffer() const
	{
		return mRenderBuffers[mCurrentBufferIndex].mBuffer;
	}


	GPUBuffer::~GPUBuffer()
	{
		mRenderService->queueVulkanObjectDestructor([buffers = mRenderBuffers, staging = mStagingBuffer](RenderService& renderService)
		{
			// Destroy staging buffer
			if(staging.mAllocation != VK_NULL_HANDLE)
				vmaDestroyBuffer(renderService.getVulkanAllocator(), staging.mBuffer, staging.mAllocation);

			// Destroy render buffers
			for (const BufferData& buffer : buffers)
			{
				if (buffer.mAllocation == VK_NULL_HANDLE)
					continue;
				vmaDestroyBuffer(renderService.getVulkanAllocator(), buffer.mBuffer, buffer.mAllocation);
			}
		});
	}

	
	bool GPUBuffer::setDataInternal(void* data, int elementSize, size_t numVertices, size_t reservedNumVertices, VkBufferUsageFlagBits usage, utility::ErrorState& error)
	{
		if (numVertices == 0)
			return true;

		// Update buffers based on selected data usage type
		switch (mUsage)
		{
			case EMeshDataUsage::DynamicWrite:
				return setDataInternalDynamic(data, elementSize, numVertices, reservedNumVertices, usage, error);
			case EMeshDataUsage::Static:
				return setDataInternalStatic(data, elementSize, numVertices, usage, error);
			default:
				assert(false);
				break;
		}
		error.fail("Unsupported buffer usage");
		return false;
	}


	bool GPUBuffer::setDataInternalStatic(void* data, int elementSize, size_t numVertices, VkBufferUsageFlagBits usage, utility::ErrorState& error)
	{
		// Calculate buffer byte size and fetch allocator
		mSize = elementSize * numVertices;
		VmaAllocator allocator = mRenderService->getVulkanAllocator();

		// Make sure we haven't already uploaded or are attempting to upload data
		if (mRenderBuffers[0].mAllocation != VK_NULL_HANDLE || mStagingBuffer.mAllocation != VK_NULL_HANDLE)
		{
			error.fail("Buffer already allocated!");
			return false;
		}

		// Create staging buffer information
		VkBufferCreateInfo stage_buffer_info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		stage_buffer_info.size = mSize;
		stage_buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		stage_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		// Create allocation info
		VmaAllocationCreateInfo stage_alloc_info = {};
		stage_alloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		stage_alloc_info.flags = 0;

		// Create staging buffer
		VkResult result = vmaCreateBuffer(allocator, &stage_buffer_info, &stage_alloc_info, &mStagingBuffer.mBuffer, &mStagingBuffer.mAllocation, &mStagingBuffer.mAllocationInfo);
		if (!error.check(result == VK_SUCCESS, "Unable to create staging buffer"))
			return false;

		// Copy data into staging buffer
		void* mapped_memory;
		result = vmaMapMemory(allocator, mStagingBuffer.mAllocation, &mapped_memory);
		assert(result == VK_SUCCESS);
		memcpy(mapped_memory, data, mSize);
		vmaUnmapMemory(allocator, mStagingBuffer.mAllocation);

		// Now create the GPU buffer to transfer data to, create buffer information
		VkBufferCreateInfo render_buffer_info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		render_buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage;
		render_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		render_buffer_info.size = mSize;

		// Create GPU buffer allocation info
		VmaAllocationCreateInfo render_alloc_info = {};
		render_alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		render_alloc_info.flags = 0;

		// Create GPU buffer
		BufferData& gpu_buffer = mRenderBuffers[0];
		result = vmaCreateBuffer(allocator, &render_buffer_info, &render_alloc_info, &gpu_buffer.mBuffer, &gpu_buffer.mAllocation, &gpu_buffer.mAllocationInfo);
		if (!error.check(result == VK_SUCCESS, "Unable to create render buffer"))
			return false;

		// Request upload
		mRenderService->requestBufferUpload(*this);
		return true;
	}


	bool GPUBuffer::setDataInternalDynamic(void* data, int elementSize, size_t numVertices, size_t reservedNumVertices, VkBufferUsageFlagBits usage, utility::ErrorState& error)
	{
		// For each update of data, we cycle through the buffers. This has the effect that if you only ever need a single buffer (static data), you 
		// will use only one buffer.
		mCurrentBufferIndex = (mCurrentBufferIndex + 1) % mRenderBuffers.size();

		// Calculate buffer byte size and fetch allocator
		uint32_t required_size_bytes = elementSize * reservedNumVertices;
		VmaAllocator allocator = mRenderService->getVulkanAllocator();

		// If we didn't allocate a buffer yet, or if the buffer has grown, we allocate it. The buffer size depends on reservedNumVertices.
		BufferData& buffer_data = mRenderBuffers[mCurrentBufferIndex];
		if (buffer_data.mBuffer == VK_NULL_HANDLE || required_size_bytes > buffer_data.mAllocationInfo.size)
		{
			// TODO: Queue destruction of buffer, could be in use
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
			if (!error.check(result == VK_SUCCESS, "Unable to create render buffer"))
				return false;
		}

		// Upload directly into buffer, use exact data size
		mSize = elementSize * numVertices;
		void* mapped_memory;
		VkResult result = vmaMapMemory(allocator, buffer_data.mAllocation, &mapped_memory);
		assert(result == VK_SUCCESS);
		memcpy(mapped_memory, data, mSize);
		vmaUnmapMemory(allocator, buffer_data.mAllocation);

		// Signal change
		bufferChanged();
		return true;
	}


	void GPUBuffer::upload(VkCommandBuffer commandBuffer)
	{
		assert(mStagingBuffer.mAllocation != VK_NULL_HANDLE);
		VkBufferCopy copyRegion{};
		copyRegion.size = mSize;
		vkCmdCopyBuffer(commandBuffer, mStagingBuffer.mBuffer, mRenderBuffers[0].mBuffer, 1, &copyRegion);
		bufferChanged();
	}
}