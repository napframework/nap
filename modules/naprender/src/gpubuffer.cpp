/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "gpubuffer.h"
#include "renderservice.h"
#include "rtti/typeinfo.h"

// External Includes
#include "vulkan/vulkan.h"
#include <nap/core.h>
#include <assert.h>
#include <string.h>
#include "renderservice.h"
#include "nap/logger.h"

RTTI_BEGIN_ENUM(nap::EMeshDataUsage)
	RTTI_ENUM_VALUE(nap::EMeshDataUsage::Static,		"Static"),
	RTTI_ENUM_VALUE(nap::EMeshDataUsage::DynamicRead,	"DynamicRead"),
	RTTI_ENUM_VALUE(nap::EMeshDataUsage::DynamicWrite,	"DynamicWrite")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBuffer)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static
	//////////////////////////////////////////////////////////////////////////

	static int getInitialNumStagingBuffers(int maxFramesInFlight, EMeshDataUsage meshUsage)
	{
		switch (meshUsage)
		{
			// When DynamicWrite, we upload into render buffers directly using mapped pointers and require no staging buffers 
		case EMeshDataUsage::DynamicWrite:
			return 0;

			// When Static, create one staging buffer for the initial upload after which it is destroyed
		case EMeshDataUsage::Static:
			return 1;

			// When DynamicRead, create one staging buffer for each frame in flight
		case EMeshDataUsage::DynamicRead:
			return maxFramesInFlight;

		default:
			assert(false);
		}
		return 0;
	}


	static void copyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkBufferCopy region = {};
		region.srcOffset = 0;
		region.dstOffset = 0;
		region.size = size;

		// Copy to staging buffer
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &region);
	}

	/**
	 * Transition image to a new layout using an existing image barrier.
	 */
	static void memoryBarrier(VkCommandBuffer commandBuffer, VkBuffer buffer, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage)
	{
		VkMemoryBarrier barrier;
		barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		barrier.srcAccessMask = srcAccessMask;
		barrier.dstAccessMask = dstAccessMask;
		barrier.pNext = VK_NULL_HANDLE;

		vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 1, &barrier, 0, nullptr, 0, nullptr);
	}


	//////////////////////////////////////////////////////////////////////////
	// GPUBuffer
	//////////////////////////////////////////////////////////////////////////

	GPUBuffer::GPUBuffer(Core& core) :
		mRenderService(core.getService<RenderService>())
	{}


	GPUBuffer::GPUBuffer(Core& core, EMeshDataUsage usage) :
		mRenderService(core.getService<RenderService>()), mUsage(usage)
	{}


	bool GPUBuffer::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mRenderBuffers.empty(), utility::stringFormat("%s: Renderbuffers created before initialization", mID.c_str())))
			return false;

		// Scale render buffers based on number of frames in flight when not static.
		mRenderBuffers.resize(mUsage == EMeshDataUsage::Static || mUsage == EMeshDataUsage::DynamicRead ? 1 : mRenderService->getMaxFramesInFlight() + 1);

		// Create appropriate number of staging buffers
		mStagingBuffers.resize(getInitialNumStagingBuffers(mRenderService->getMaxFramesInFlight(), mUsage));

		// Ensure there are enough read callbacks based on max number of frames in flight
		if (mUsage == EMeshDataUsage::DynamicRead)
		{
			mReadCallbacks.resize(mRenderService->getMaxFramesInFlight());
			mDownloadStagingBufferIndices.resize(mRenderService->getMaxFramesInFlight());
		}

		return true;
	}


	bool GPUBuffer::allocateInternal(int elementSize, size_t numVertices, size_t reservedNumVertices, VkBufferUsageFlagBits usage, utility::ErrorState& errorState)
	{
		return allocateInternal(elementSize * numVertices, usage, errorState);
	}


	bool GPUBuffer::allocateInternal(size_t size, VkBufferUsageFlagBits usage, utility::ErrorState& errorState)
	{
		// Persistent storage
		if (mUsage == EMeshDataUsage::Static || mUsage == EMeshDataUsage::DynamicRead)
		{
			// Calculate buffer byte size and fetch allocator
			VmaAllocator allocator = mRenderService->getVulkanAllocator();

			// Make sure we haven't already uploaded or are attempting to upload data
			bool staging_buffers_free = true;
			for (auto& staging_buffer : mStagingBuffers)
				staging_buffers_free &= staging_buffer.mBuffer != VK_NULL_HANDLE;

			if (mRenderBuffers[0].mBuffer != VK_NULL_HANDLE || staging_buffers_free)
			{
				errorState.fail("Attempting to upload data to previously allocated buffer");
				errorState.fail("Not allowed when usage is static");
				return false;
			}

			// When read frequently, the buffer is a destination, otherwise used as a source for texture upload
			VkBufferUsageFlags buffer_usage = mUsage == EMeshDataUsage::DynamicRead ?
				VK_BUFFER_USAGE_TRANSFER_DST_BIT :
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

			// When read frequently, the buffer receives from the GPU, otherwise the buffer receives from CPU
			VmaMemoryUsage memory_usage = mUsage == EMeshDataUsage::DynamicRead ?
				VMA_MEMORY_USAGE_GPU_TO_CPU :
				VMA_MEMORY_USAGE_CPU_TO_GPU;

			for (auto& staging_buffer : mStagingBuffers)
			{
				// Create staging buffer
				if (!createBuffer(allocator, size, buffer_usage, memory_usage, 0, staging_buffer, errorState))
				{
					errorState.fail("Unable to create staging buffer");
					return false;
				}
			}

			// Device buffer memory usage
			VkBufferUsageFlags gpu_buffer_usage = mUsage == EMeshDataUsage::DynamicRead ?
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT :
				VK_BUFFER_USAGE_TRANSFER_DST_BIT;

			// Now create the GPU buffer to transfer data to, create buffer information
			if (!createBuffer(allocator, size, usage | gpu_buffer_usage, VMA_MEMORY_USAGE_GPU_ONLY, 0, mRenderBuffers[0], errorState))
			{
				errorState.fail("Unable to create render buffer");
				return false;
			}

			// Cache buffer size
			mSize = size;
		}
		return true;
	}


	VkBuffer GPUBuffer::getBuffer() const
	{
		return mRenderBuffers[mCurrentRenderBufferIndex].mBuffer;
	}


	const BufferData& GPUBuffer::getBufferData() const
	{
		return mRenderBuffers[mCurrentRenderBufferIndex];
	}

	
	bool GPUBuffer::setDataInternal(void* data, int elementSize, size_t numVertices, size_t reservedNumVertices, VkBufferUsageFlagBits usage, utility::ErrorState& errorState)
	{
		// Skip if the size of the buffer to upload is zero
		if (numVertices == 0)
			return true;

		return setDataInternal(data, elementSize * numVertices, elementSize * reservedNumVertices, usage, errorState);
	}


	bool GPUBuffer::setDataInternal(void* data, size_t size, size_t reservedSize, VkBufferUsageFlagBits usage, utility::ErrorState& errorState)
	{
		// Ensure the buffer isn't DynamicRead
		if (!errorState.check(mUsage != EMeshDataUsage::DynamicRead, "DynamicRead buffers cannot be written to"))
			return false;

		// Allocate the buffers if necessary
		assert(!mRenderBuffers.empty());
		if (mUsage != EMeshDataUsage::DynamicWrite && mRenderBuffers[0].mBuffer == VK_NULL_HANDLE)
		{
			if (!allocateInternal(size, usage, errorState))
				return false;
		}

		// Update buffers based on selected data usage type
		switch (mUsage)
		{
		case EMeshDataUsage::DynamicWrite:
			return setDataInternalDynamic(data, size, reservedSize, usage, errorState);
		case EMeshDataUsage::Static:
			return setDataInternalStatic(data, size, usage, errorState);
		default:
			assert(false);
			break;
		}
		errorState.fail("Unsupported buffer usage");
		return false;
	}


	bool GPUBuffer::setDataInternalStatic(void* data, size_t size, VkBufferUsageFlagBits usage, utility::ErrorState& errorState)
	{
		// When usage is static, the staging buffer is released after the upload
		if (mStagingBuffers[0].mBuffer == VK_NULL_HANDLE)
		{
			errorState.fail("Attempting to upload data to previously allocated buffer");
			errorState.fail("Not allowed when usage is static");
			return false;
		}

		// Verify upload size
		if (!errorState.check(size <= mSize, "Buffer upload size exceeds allocated size"))
			return false;

		// Calculate buffer byte size and fetch allocator
		VmaAllocator allocator = mRenderService->getVulkanAllocator();
		for (auto& staging_buffer : mStagingBuffers)
		{
			// Copy data into staging buffer
			if (!errorState.check(uploadToBuffer(allocator, size, data, staging_buffer), "Unable to upload data to staging buffer"))
				return false;
		}

		// Request upload
		mRenderService->requestBufferUpload(*this);
		return true;
	}


	bool GPUBuffer::setDataInternalDynamic(void* data, size_t size, size_t reservedSize, VkBufferUsageFlagBits usage, utility::ErrorState& errorState)
	{
		// For each update of data, we cycle through the buffers
		mCurrentRenderBufferIndex = (mCurrentRenderBufferIndex + 1) % mRenderBuffers.size();

		// Fetch allocator
		VmaAllocator allocator = mRenderService->getVulkanAllocator();

		// If we didn't allocate a buffer yet, or if the buffer has grown, we allocate it. 
		// The final buffer size is calculated based on the reservedNumVertices.
		BufferData& buffer_data = mRenderBuffers[mCurrentRenderBufferIndex];
		if (buffer_data.mBuffer == VK_NULL_HANDLE || reservedSize > buffer_data.mAllocationInfo.size)
		{
			// Queue buffer for destruction if already allocated, the buffer data is copied, not captured by reference.
			if (buffer_data.mBuffer != VK_NULL_HANDLE)
			{
				mRenderService->queueVulkanObjectDestructor([buffer = buffer_data](RenderService& renderService)
				{
					destroyBuffer(renderService.getVulkanAllocator(), buffer);
				});
			}

			// Create new buffer
			if (!createBuffer(allocator, reservedSize, usage, VMA_MEMORY_USAGE_CPU_TO_GPU, 0, buffer_data, errorState))
			{
				errorState.fail("Render buffer error");
				return false;
			}
		}

		// Upload directly into buffer, use exact data size
		if (!errorState.check(uploadToBuffer(allocator, size, data, buffer_data), "Buffer upload failed"))
			return false;

		// Cache buffer size
		mSize = size;

		bufferChanged();
		return true;
	}


	void GPUBuffer::upload(VkCommandBuffer commandBuffer)
	{
		// Ensure we're dealing with an empty buffer, size of 1 that is used static.
		assert(mStagingBuffers.size() > 0 && mStagingBuffers[0].mBuffer != VK_NULL_HANDLE);
		assert(mRenderBuffers.size() == 1);
		assert(mUsage == EMeshDataUsage::Static || mUsage == EMeshDataUsage::DynamicRead);
		
		// Copy staging buffer to GPU
		VkBufferCopy copyRegion = {};
		copyRegion.size = mSize;
		vkCmdCopyBuffer(commandBuffer, mStagingBuffers[0].mBuffer, mRenderBuffers[0].mBuffer, 1, &copyRegion);

		// Queue destruction of staging buffer if usage is static
		// This queues the vulkan staging resource for destruction, executed by the render service at the appropriate time.
		// Explicitly release the buffer, so it's not deleted twice
		if (mUsage == EMeshDataUsage::Static)
		{
			mRenderService->queueVulkanObjectDestructor([staging_buffers = mStagingBuffers](RenderService & renderService)
			{
				for (const BufferData& buffer : staging_buffers)
					destroyBuffer(renderService.getVulkanAllocator(), buffer);
			});
			for (BufferData& buffer : mStagingBuffers)
				buffer.release();
		}

		// Signal change
		bufferChanged();
	}


	void GPUBuffer::download(VkCommandBuffer commandBuffer)
	{
		assert(mUsage == EMeshDataUsage::DynamicRead);
		BufferData& staging_buffer = mStagingBuffers[mCurrentStagingBufferIndex];

		// Store the staging buffer index associated with the download in the current frame for lookup later
		mDownloadStagingBufferIndices[mRenderService->getCurrentFrameIndex()] = mCurrentStagingBufferIndex;
		mCurrentStagingBufferIndex = (mCurrentStagingBufferIndex + 1) % mStagingBuffers.size();

		VkBufferCopy region = {};
		region.srcOffset = 0;
		region.dstOffset = 0;
		region.size = staging_buffer.mAllocationInfo.size;

		VkPipelineStageFlags stage_flags = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

		// Set memory barriers
		memoryBarrier(commandBuffer, staging_buffer.mBuffer, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, stage_flags, VK_PIPELINE_STAGE_TRANSFER_BIT);

		// Copy to staging buffer
		copyBuffer(commandBuffer, mRenderBuffers[0].mBuffer, staging_buffer.mBuffer, staging_buffer.mAllocationInfo.size);

		// Complete execution dependency
		memoryBarrier(commandBuffer, staging_buffer.mBuffer, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, stage_flags);
	}


	void GPUBuffer::notifyDownloadReady(int frameIndex)
	{
		// Update the staging buffer using the Bitmap contents
		VmaAllocator vulkan_allocator = mRenderService->getVulkanAllocator();

		// Copy data, not for this to work the VK_MEMORY_PROPERTY_HOST_COHERENT_BIT is required on OSX!
		int downloaded_staging_buffer_index = mDownloadStagingBufferIndices[frameIndex];
		BufferData& buffer = mStagingBuffers[downloaded_staging_buffer_index];

		void* mapped_memory = nullptr;
		VkResult result = vmaMapMemory(vulkan_allocator, buffer.mAllocation, &mapped_memory);
		assert(result == VK_SUCCESS);

		mReadCallbacks[frameIndex](mapped_memory, mSize);
		vmaUnmapMemory(vulkan_allocator, buffer.mAllocation);
		mReadCallbacks[frameIndex] = BufferReadCallback();
	}


	void GPUBuffer::asyncGetData(std::function<void(const void*, size_t)> copyFunction)
	{
		assert(!mReadCallbacks[mRenderService->getCurrentFrameIndex()]);
		mReadCallbacks[mRenderService->getCurrentFrameIndex()] = copyFunction;
		mRenderService->requestBufferDownload(*this);
	}


	GPUBuffer::~GPUBuffer()
	{
		// Queue buffers for destruction, the buffer data is copied, not captured by reference.
		// This ensures the buffers are destroyed when certain they are not in use.
		mRenderService->removeBufferRequests(*this);
		mRenderService->queueVulkanObjectDestructor([render_buffers = mRenderBuffers, staging_buffers = mStagingBuffers](RenderService& renderService)
		{
			// Destroy render buffers
			for (const BufferData& buffer : render_buffers)
				destroyBuffer(renderService.getVulkanAllocator(), buffer);

			// Also destroy the staging buffers if we reach this point before the initial upload has occurred.
			// This could happen e.g. if app initialization fails.
			for (const BufferData& buffer : staging_buffers)
				destroyBuffer(renderService.getVulkanAllocator(), buffer);
		});
	}
}
