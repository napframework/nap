/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "gpudatabuffer.h"
#include "renderservice.h"

// External Includes
#include "vulkan/vulkan.h"
#include <nap/core.h>

namespace nap
{
	GPUDataBuffer::GPUDataBuffer(RenderService& renderService, EBufferObjectType type) :
		mRenderService(&renderService), mType(type)
	{ }


	bool GPUDataBuffer::setData(uint32 size, uint8* data, utility::ErrorState& errorState)
	{
		VmaAllocator allocator = mRenderService->getVulkanAllocator();

		// Make sure we haven't already uploaded or are attempting to upload data
		if (mStagingBuffer.mBuffer != VK_NULL_HANDLE)
		{
			errorState.fail("Attempting to upload data to previously allocated buffer.");
			return false;
		}

		// Create staging buffer
		if (!createBuffer(allocator, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, 0, mStagingBuffer, errorState))
		{
			errorState.fail("Unable to create staging buffer");
			return false;
		}
		mSize = size;

		// Copy data into staging buffer
		if (!errorState.check(uploadToBuffer(allocator, size, data, mStagingBuffer), "Unable to upload data to staging buffer"))
			return false;

		return true;
	}


	/**
	 *
	 */
	bool GPUDataBuffer::getData(BufferData& outBufferData, utility::ErrorState& errorState)
	{
		VmaAllocator allocator = mRenderService->getVulkanAllocator();

		// Now create the GPU buffer to transfer data to, create buffer information
		if (!createBuffer(allocator, mSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | getBufferUsage(mType), VMA_MEMORY_USAGE_GPU_ONLY, 0, outBufferData, errorState))
		{
			errorState.fail("Unable to create render buffer");
			return false;
		}

		mDeviceBuffer = &outBufferData;

		// Request upload
		mRenderService->requestBufferUpload(*this);
		return true;
	}


	void GPUDataBuffer::upload(VkCommandBuffer commandBuffer)
	{
		// Ensure we're dealing with an empty buffer, size of 1 that is used static.
		assert(mStagingBuffer.mBuffer != VK_NULL_HANDLE);
		assert(mDeviceBuffer != nullptr);

		// Copy staging buffer to GPU
		VkBufferCopy copyRegion = {};
		copyRegion.size = mSize;
		vkCmdCopyBuffer(commandBuffer, mStagingBuffer.mBuffer, mDeviceBuffer->mBuffer, 1, &copyRegion);

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


	GPUDataBuffer::~GPUDataBuffer()
	{
		// Queue buffers for destruction, the buffer data is copied, not captured by reference.
		// This ensures the buffers are destroyed when certain they are not in use.
		mRenderService->removeBufferRequests(*this);
		mRenderService->queueVulkanObjectDestructor([staging_buffer = mStagingBuffer](RenderService& renderService)
		{
			// Also destroy the staging buffer if we reach this point before the initial upload has occurred.
			// This could happen e.g. if app initialization fails.
			destroyBuffer(renderService.getVulkanAllocator(), staging_buffer);
		});
	}
}
