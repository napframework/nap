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
	RTTI_ENUM_VALUE(nap::EMeshDataUsage::DynamicWrite,	"DynamicWrite"),
	RTTI_ENUM_VALUE(nap::EMeshDataUsage::DynamicRead,	"DynamicRead")
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
		// When DynamicWrite, we upload to render buffers directly and require no staging buffers 
		case EMeshDataUsage::DynamicWrite:
			return 0;

		// When Static, create one staging buffer for the initial upload and destroy later
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

	
	bool GPUBuffer::setDataInternal(void* data, int elementSize, size_t numVertices, size_t reservedNumVertices, VkBufferUsageFlagBits usage, utility::ErrorState& error)
	{
		if (numVertices == 0)
			return true;

		// Cache buffer size
		mSize = elementSize * numVertices;

		// Update buffers based on selected data usage type
		switch (mUsage)
		{
		case EMeshDataUsage::DynamicWrite:
			return setDataInternalDynamic(data, mSize, elementSize * reservedNumVertices, usage, error);
		case EMeshDataUsage::DynamicRead:
		case EMeshDataUsage::Static:
			return setDataInternalStatic(data, mSize, usage, error);
		default:
			assert(false);
			break;
		}
		error.fail("Unsupported buffer usage");
		return false;
	}


	bool GPUBuffer::setDataInternal(void* data, size_t size, VkBufferUsageFlagBits usage, utility::ErrorState& error)
	{
		// Cache buffer size
		mSize = size;

		// Update buffers based on selected data usage type
		switch (mUsage)
		{
		case EMeshDataUsage::DynamicWrite:
			return setDataInternalDynamic(data, size, size, usage, error);
		case EMeshDataUsage::DynamicRead:
		case EMeshDataUsage::Static:
			return setDataInternalStatic(data, size, usage, error);
		default:
			assert(false);
			break;
		}
		error.fail("Unsupported buffer usage");
		return false;
	}


	bool GPUBuffer::setDataInternalStatic(void* data, size_t size, VkBufferUsageFlagBits usage, utility::ErrorState& error)
	{
		// Calculate buffer byte size and fetch allocator
		VmaAllocator allocator = mRenderService->getVulkanAllocator();

		// Make sure we haven't already uploaded or are attempting to upload data
		bool staging_buffers_free = true;
		for (auto& staging_buffer : mStagingBuffers)
			staging_buffers_free &= staging_buffer.mBuffer != VK_NULL_HANDLE;

		if (mRenderBuffers[0].mBuffer != VK_NULL_HANDLE || staging_buffers_free)
		{
			error.fail("Attempting to upload data to previously allocated buffer.");
			error.fail("Not allowed when usage is static or dynamicread");
			return false;
		}

		for (auto& staging_buffer : mStagingBuffers)
		{
			// Create staging buffer
			if (!createBuffer(allocator, mSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, 0, staging_buffer, error))
			{
				error.fail("Unable to create staging buffer");
				return false;
			}

			// Copy data into staging buffer
			if (!error.check(uploadToBuffer(allocator, mSize, data, staging_buffer), "Unable to upload data to staging buffer"))
				return false;
		}

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


	bool GPUBuffer::setDataInternalDynamic(void* data, size_t size, size_t reservedSize, VkBufferUsageFlagBits usage, utility::ErrorState& error)
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
			if (!createBuffer(allocator, reservedSize, usage, VMA_MEMORY_USAGE_CPU_TO_GPU, 0, buffer_data, error))
			{
				error.fail("Render buffer error");
				return false;
			}
		}

		// Upload directly into buffer, use exact data size
		if (!error.check(uploadToBuffer(allocator, mSize, data, buffer_data), "Buffer upload failed"))
			return false;
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
		BufferData& buffer = mStagingBuffers[mCurrentStagingBufferIndex];

		// Store the staging buffer index associated with the download in the current frame for lookup later
		mDownloadStagingBufferIndices[mRenderService->getCurrentFrameIndex()] = mCurrentStagingBufferIndex;
		mCurrentStagingBufferIndex = (mCurrentStagingBufferIndex + 1) % mStagingBuffers.size();

		//// Transition for copy
		//transitionImageLayout(commandBuffer, mImageData.mTextureImage,
		//	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		//	VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
		//	VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		//	0, 1);

		//// Copy to buffer
		//copyImageToBuffer(commandBuffer, mImageData.mTextureImage, buffer.mBuffer, mDescriptor.mWidth, mDescriptor.mHeight);

		//// Transition back to shader usage
		//transitionImageLayout(commandBuffer, mImageData.mTextureImage,
		//	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		//	VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT,
		//	VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		//	0, 1);
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
