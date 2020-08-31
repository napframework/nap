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
		// Queue buffers for destruction, the buffer data is copied, not captured by reference.
		// This ensures the buffers are destroyed when certain they are not in use.
		mRenderService->removeBufferRequests(*this);
		mRenderService->queueVulkanObjectDestructor([buffers = mRenderBuffers](RenderService& renderService)
		{
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
			error.fail("Attempting to upload data to previously allocated buffer.");
			error.fail("Not allowed when usage is static");
			return false;
		}

		// Create staging buffer
		if (!createBuffer(allocator, mSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, 0, mStagingBuffer, error))
		{
			error.fail("Unable to create staging buffer");
			return false;
		}
			
		// Copy data into staging buffer
		if (!error.check(uploadToBuffer(allocator, mSize, data, mStagingBuffer), "Unable to upload data to staging buffer"))
			return false;

		// Now create the GPU buffer to transfer data to, create buffer information
		if (!createBuffer(allocator, mSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VMA_MEMORY_USAGE_GPU_ONLY, 0, mRenderBuffers[0], error))
		{
			error.fail("Unable to create render buffer");
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

		// If we didn't allocate a buffer yet, or if the buffer has grown, we allocate it. 
		// The final buffer size is calculated based on the reservedNumVertices.
		BufferData& buffer_data = mRenderBuffers[mCurrentBufferIndex];
		if (buffer_data.mBuffer == VK_NULL_HANDLE || required_size_bytes > buffer_data.mAllocationInfo.size)
		{
			// Queue buffer for destruction if already allocated, the buffer data is copied, not captured by reference.
			if (buffer_data.mBuffer != VK_NULL_HANDLE)
			{
				mRenderService->queueVulkanObjectDestructor([buffer = buffer_data](RenderService& renderService)
				{
					destroyBuffer(renderService.getVulkanAllocator(), buffer);
				});
			}

			// Create buffer new buffer
			if (!createBuffer(allocator, required_size_bytes, usage, VMA_MEMORY_USAGE_CPU_TO_GPU, 0, buffer_data, error))
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
		// Ensure we're dealing with an empty buffer, size of 1 that is used static.
		assert(mStagingBuffer.mBuffer != VK_NULL_HANDLE);
		assert(mUsage == EMeshDataUsage::Static);
		assert(mRenderBuffers.size() == 1);
		
		// Copy staging buffer to GPU
		VkBufferCopy copyRegion = {};
		copyRegion.size = mSize;
		vkCmdCopyBuffer(commandBuffer, mStagingBuffer.mBuffer, mRenderBuffers[0].mBuffer, 1, &copyRegion);

		// Queue destruction of staging buffer
		// This queues the vulkan staging resource for destruction, executed by the render service at the appropriate time.
		// Explicitly release the buffer, so it's not deleted twice
		mRenderService->queueVulkanObjectDestructor([buffer = mStagingBuffer](RenderService& renderService)
		{
			destroyBuffer(renderService.getVulkanAllocator(), buffer);
		});
		mStagingBuffer.release();

		// Signal change
		bufferChanged();
	}
}
