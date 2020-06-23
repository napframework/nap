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
	//////////////////////////////////////////////////////////////////////////
	// Static functions
	//////////////////////////////////////////////////////////////////////////

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
			destroyBuffer(renderService.getVulkanAllocator(), staging);

			// Destroy render buffers
			for (const BufferData& buffer : buffers)
				destroyBuffer(renderService.getVulkanAllocator(), buffer);
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
		if (mRenderBuffers[0].mBuffer != VK_NULL_HANDLE || mStagingBuffer.mBuffer != VK_NULL_HANDLE)
		{
			error.fail("Buffer already allocated!");
			return false;
		}

		// Create staging buffer
		if (!createBuffer(allocator, mSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, mStagingBuffer, error))
		{
			error.fail("Staging buffer error");
			return false;
		}
			
		// Copy data into staging buffer
		if (!error.check(uploadToBuffer(allocator, mSize, data, mStagingBuffer), "Unable to upload data to staging buffer"))
			return false;

		// Now create the GPU buffer to transfer data to, create buffer information
		if (!createBuffer(allocator, mSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VMA_MEMORY_USAGE_GPU_ONLY, mRenderBuffers[0], error))
		{
			error.fail("Render buffer error");
			return false;
		}

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
			// Destroy buffer if allocated
			// TODO: Queue destruction of buffer, could be in use
			destroyBuffer(allocator, buffer_data);

			// Create buffer
			if (!createBuffer(allocator, required_size_bytes, usage, VMA_MEMORY_USAGE_CPU_TO_GPU, buffer_data, error))
			{
				error.fail("Render buffer error");
				return false;
			}
		}

		// Upload directly into buffer, use exact data size
		mSize = elementSize * numVertices;
		if (!error.check(uploadToBuffer(allocator, mSize, data, buffer_data), "Buffer upload failed"))
			return false;
		bufferChanged();

		return true;
	}


	void GPUBuffer::upload(VkCommandBuffer commandBuffer)
	{
		// Copy staging buffer to GPU
		assert(mStagingBuffer.mAllocation != VK_NULL_HANDLE);
		VkBufferCopy copyRegion{};
		copyRegion.size = mSize;
		vkCmdCopyBuffer(commandBuffer, mStagingBuffer.mBuffer, mRenderBuffers[0].mBuffer, 1, &copyRegion);

		// Signal change
		bufferChanged();
	}
}