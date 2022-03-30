/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "basegpubuffer.h"
#include "renderservice.h"
#include "rtti/typeinfo.h"

// External Includes
#include "vulkan/vulkan.h"
#include "renderservice.h"
#include "nap/logger.h"

#include <nap/core.h>
#include <assert.h>
#include <string.h>

RTTI_BEGIN_ENUM(nap::EMemoryUsage)
	RTTI_ENUM_VALUE(nap::EMemoryUsage::Static,			"Static"),
	RTTI_ENUM_VALUE(nap::EMemoryUsage::DynamicRead,		"DynamicRead"),
	RTTI_ENUM_VALUE(nap::EMemoryUsage::DynamicWrite,	"DynamicWrite")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BaseGPUBuffer)
	RTTI_PROPERTY("Clear",				&nap::BaseGPUBuffer::mClear,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage",				&nap::BaseGPUBuffer::mUsage,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static
	//////////////////////////////////////////////////////////////////////////

	static int getInitialNumStagingBuffers(int maxFramesInFlight, EMemoryUsage meshUsage)
	{
		switch (meshUsage)
		{
			// When DynamicWrite, we upload into render buffers directly using mapped pointers and require no staging buffers 
		case EMemoryUsage::DynamicWrite:
			return 0;

			// When Static, create one staging buffer for the initial upload after which it is destroyed
		case EMemoryUsage::Static:
			return 1;

			// When DynamicRead, create one staging buffer for each frame in flight
		case EMemoryUsage::DynamicRead:
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

	BaseGPUBuffer::BaseGPUBuffer(Core& core) :
		mRenderService(core.getService<RenderService>())
	{}


	BaseGPUBuffer::BaseGPUBuffer(Core& core, EMemoryUsage usage) :
		mRenderService(core.getService<RenderService>()), mUsage(usage)
	{}


	bool BaseGPUBuffer::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mRenderBuffers.empty(), utility::stringFormat("%s: Renderbuffers created before initialization", mID.c_str())))
			return false;

		// Scale render buffers based on number of frames in flight when not static.
		mRenderBuffers.resize(mUsage == EMemoryUsage::Static || mUsage == EMemoryUsage::DynamicRead ? 1 : mRenderService->getMaxFramesInFlight() + 1);

		// Create appropriate number of staging buffers
		mStagingBuffers.resize(getInitialNumStagingBuffers(mRenderService->getMaxFramesInFlight(), mUsage));

		// Ensure there are enough read callbacks based on max number of frames in flight
		if (mUsage == EMemoryUsage::DynamicRead)
		{
			mReadCallbacks.resize(mRenderService->getMaxFramesInFlight());
			mDownloadStagingBufferIndices.resize(mRenderService->getMaxFramesInFlight());
		}

		return true;
	}


	bool BaseGPUBuffer::allocateInternal(size_t size, VkBufferUsageFlags deviceUsage, utility::ErrorState& errorState)
	{
		// Persistent storage
		if (mUsage == EMemoryUsage::DynamicWrite)
		{
			mSize = size;
			return true;
		}
		else
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
			VkBufferUsageFlags buffer_usage = mUsage == EMemoryUsage::DynamicRead ?
				VK_BUFFER_USAGE_TRANSFER_DST_BIT :
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

			// When read frequently, the buffer receives from the GPU, otherwise the buffer receives from CPU
			VmaMemoryUsage memory_usage = mUsage == EMemoryUsage::DynamicRead ?
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

			// Update buffer size
			if (!mStagingBuffers.empty())
				mSize = size;

			// Device buffer memory usage
			VkBufferUsageFlags transfer_usage = mUsage == EMemoryUsage::DynamicRead ?
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT :
				VK_BUFFER_USAGE_TRANSFER_DST_BIT;

			// Make sure the transfer bits are not set in advance as these are determined from the mesh data usage property
			deviceUsage &= ~(VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

			// Now create the GPU buffer to transfer data to, create buffer information
			if (!createBuffer(allocator, size, deviceUsage | transfer_usage, VMA_MEMORY_USAGE_GPU_ONLY, 0, mRenderBuffers[0], errorState))
			{
				errorState.fail("Unable to create render buffer");
				return false;
			}
		}
		return true;
	}


	VkBuffer BaseGPUBuffer::getBuffer() const
	{
		return mRenderBuffers[mCurrentRenderBufferIndex].mBuffer;
	}


	const BufferData& BaseGPUBuffer::getBufferData() const
	{
		return mRenderBuffers[mCurrentRenderBufferIndex];
	}


	bool BaseGPUBuffer::setDataInternal(const void* data, size_t size, size_t reservedSize, VkBufferUsageFlags deviceUsage, utility::ErrorState& errorState)
	{
		// Ensure the sizes are valid
		assert(size <= reservedSize || size > 0);

		// Ensure the buffer isn't DynamicRead
		if (!errorState.check(mUsage != EMemoryUsage::DynamicRead, "DynamicRead buffers cannot be written to"))
			return false;

		// Update buffers based on selected data usage type
		switch (mUsage)
		{
		case EMemoryUsage::DynamicWrite:
			return setDataInternalDynamic(data, size, reservedSize, deviceUsage, errorState);
		case EMemoryUsage::Static:
			return setDataInternalStatic(data, size, errorState);
		default:
			assert(false);
			break;
		}
		errorState.fail("Unsupported buffer usage");
		return false;
	}


	bool BaseGPUBuffer::setDataInternalStatic(const void* data, size_t size, utility::ErrorState& errorState)
	{
		// Ensure the buffers are allocated
		assert(!mRenderBuffers.empty());
		assert(mRenderBuffers[0].mBuffer != VK_NULL_HANDLE);

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


	bool BaseGPUBuffer::setDataInternalDynamic(const void* data, size_t size, size_t reservedSize, VkBufferUsageFlags deviceUsage, utility::ErrorState& errorState)
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
			if (!createBuffer(allocator, reservedSize, deviceUsage, VMA_MEMORY_USAGE_CPU_TO_GPU, 0, buffer_data, errorState))
			{
				errorState.fail("Render buffer error");
				return false;
			}
		}

		// Cache buffer size
		mSize = size;

		// Upload directly into buffer, use exact data size
		if (!errorState.check(uploadToBuffer(allocator, size, data, buffer_data), "Buffer upload failed"))
			return false;

		bufferChanged();
		return true;
	}


	void BaseGPUBuffer::upload(VkCommandBuffer commandBuffer)
	{
		// Ensure we're dealing with an empty buffer, size of 1 that is used static.
		assert(mStagingBuffers.size() > 0 && mStagingBuffers[0].mBuffer != VK_NULL_HANDLE);
		assert(mRenderBuffers.size() == 1);
		assert(mUsage == EMemoryUsage::Static || mUsage == EMemoryUsage::DynamicRead);

		// Buffer uploads are recorded after clear commands, and we do not want operations on the same buffer to interfere with each other
		// Therefore, a buffer copy requires synchronization with a potential prior clear command
		memoryBarrier(commandBuffer, mRenderBuffers[0].mBuffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		// Copy staging buffer to GPU
		VkBufferCopy copyRegion = {};
		copyRegion.size = mSize;
		vkCmdCopyBuffer(commandBuffer, mStagingBuffers[0].mBuffer, mRenderBuffers[0].mBuffer, 1, &copyRegion);

		// Determine dest access flags for memory barrier
		VkBufferUsageFlags usage = getBufferUsageFlags();
		VkAccessFlags dst_access = 0;
		
		dst_access |= (usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) ? VK_ACCESS_INDEX_READ_BIT : 0;
		dst_access |= (usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) ? VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT : 0;
		dst_access |= (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT || usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) ? VK_ACCESS_UNIFORM_READ_BIT : 0;

		// As GPU buffers can be bound to a descriptorset for any shader stage, and we do not store information about the pipeline stages in which they will be used,
		// we assume the earliest possible pipeline stage for the buffer to be used; VERTEX and COMPUTE.
		VkPipelineStageFlags dst_stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		dst_stage |= (usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT || usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) ? VK_PIPELINE_STAGE_VERTEX_INPUT_BIT : 0;
		dst_stage |= (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT || usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) ? VK_PIPELINE_STAGE_VERTEX_SHADER_BIT : 0;

		// Memory barrier
		memoryBarrier(commandBuffer, mRenderBuffers[0].mBuffer, VK_ACCESS_TRANSFER_WRITE_BIT, dst_access, VK_PIPELINE_STAGE_TRANSFER_BIT, dst_stage);

		// Queue destruction of staging buffer if usage is static
		// This queues the vulkan staging resource for destruction, executed by the render service at the appropriate time.
		// Explicitly release the buffer, so it's not deleted twice
		if (mUsage == EMemoryUsage::Static)
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


	void BaseGPUBuffer::download(VkCommandBuffer commandBuffer)
	{
		assert(mUsage == EMemoryUsage::DynamicRead);
		BufferData& staging_buffer = mStagingBuffers[mCurrentStagingBufferIndex];

		// Store the staging buffer index associated with the download in the current frame for lookup later
		mDownloadStagingBufferIndices[mRenderService->getCurrentFrameIndex()] = mCurrentStagingBufferIndex;
		mCurrentStagingBufferIndex = (mCurrentStagingBufferIndex + 1) % mStagingBuffers.size();

		VkPipelineStageFlags stage_flags = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

		// Memory barrier
		memoryBarrier(commandBuffer, staging_buffer.mBuffer, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, stage_flags, VK_PIPELINE_STAGE_TRANSFER_BIT);

		// Copy to staging buffer
		copyBuffer(commandBuffer, mRenderBuffers[0].mBuffer, staging_buffer.mBuffer, mSize);

		// Downloads always happen at the end of a frame. Therefore, we do not need to place an additional barrier for an execution dependency
		// We can just wait for the rendering finished semaphore
	}


	void BaseGPUBuffer::clear(VkCommandBuffer commandBuffer)
	{
		// Ensure the render buffers are created
		assert(mRenderBuffers.size() > 0);

		// The vkCmdFillBuffer/clear command is treated as a TRANSFER operation
		// Clear commands are recorded to the upload command buffer and always happen at the beginning of a frame
		// Therefore, no initial memory barrier is required
		vkCmdFillBuffer(commandBuffer, mRenderBuffers[0].mBuffer, 0, VK_WHOLE_SIZE, 0);

		// Determine dest access flags for memory barrier
		VkBufferUsageFlags usage = getBufferUsageFlags();
		VkAccessFlags dst_access = 0;
		dst_access |= (usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) ? VK_ACCESS_INDEX_READ_BIT : 0;
		dst_access |= (usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) ? VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT : 0;
		dst_access |= (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT || usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) ? VK_ACCESS_UNIFORM_READ_BIT : 0;

		// As gpu buffers can be bound to a descriptorset for any shader stage, and we do not store information about the pipeline stages in which they will be used,
		// we assume the earliest possible pipeline stage for the buffer to be used - VERTEX and COMPUTE.
		VkPipelineStageFlags dst_stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		dst_stage |= (usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT || usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) ? VK_PIPELINE_STAGE_VERTEX_INPUT_BIT : 0;
		dst_stage |= (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT || usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) ? VK_PIPELINE_STAGE_VERTEX_SHADER_BIT : 0;

		// Memory barrier
		memoryBarrier(commandBuffer, mRenderBuffers[0].mBuffer, VK_ACCESS_TRANSFER_WRITE_BIT, dst_access, VK_PIPELINE_STAGE_TRANSFER_BIT, dst_stage);
	}


	void BaseGPUBuffer::notifyDownloadReady(int frameIndex)
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


	void BaseGPUBuffer::asyncGetData(std::function<void(const void*, size_t)> copyFunction)
	{
		assert(!mReadCallbacks[mRenderService->getCurrentFrameIndex()]);
		mReadCallbacks[mRenderService->getCurrentFrameIndex()] = copyFunction;
		mRenderService->requestBufferDownload(*this);
	}


	void BaseGPUBuffer::clearDownloads()
	{
		for (auto& callback : mReadCallbacks)
			callback = BufferReadCallback();
	}


	void BaseGPUBuffer::requestClear()
	{
		mRenderService->requestBufferClear(*this);
	}


	BaseGPUBuffer::~BaseGPUBuffer()
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
