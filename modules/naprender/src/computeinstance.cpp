/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "computeinstance.h"
#include "descriptorsetcache.h"

namespace nap
{
	ComputeInstance::ComputeInstance(ComputeMaterialInstanceResource& computeMaterialInstanceResource, RenderService* renderService) :
		mComputeMaterialInstanceResource(&computeMaterialInstanceResource), mRenderService(renderService)
	{}


	bool ComputeInstance::init(utility::ErrorState& errorState)
	{
		// If compute is enabled, create the compute command buffer
		if (!errorState.check(mRenderService->isComputeAvailable(), "Failed to create compute instance! Compute is unavailable."))
			return false;

		// Create command buffer
		VkCommandBufferAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool = mRenderService->getCommandPool();
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = 1;

		if (!errorState.check(vkAllocateCommandBuffers(mRenderService->getDevice(), &alloc_info, &mComputeCommandBuffer) == VK_SUCCESS, "Failed to allocate command buffer"))
			return false;

		// Create semaphore
		VkSemaphoreCreateInfo semaphore_create_info = {};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphore_create_info.pNext = nullptr;
		semaphore_create_info.flags = 0;

		if (!errorState.check(vkCreateSemaphore(mRenderService->getDevice(), &semaphore_create_info, nullptr, &mSemaphore) == VK_SUCCESS, "Failed to create semaphore"))
			return false;

		// Create fence
		VkFenceCreateInfo fence_create_info = {};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.pNext = nullptr;
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (!errorState.check(vkCreateFence(mRenderService->getDevice(), &fence_create_info, nullptr, &mFence) == VK_SUCCESS, "Failed to create fence"))
			return false;

		// Initialize compute material instance
		if (!errorState.check(mComputeMaterialInstance.init(*mRenderService, *mComputeMaterialInstanceResource, errorState), "Failed to init compute material instannce"))
			return false;

		return true;
	}


	bool ComputeInstance::asyncCompute(uint numInvocations, VkPipelineStageFlags graphicsStageFlags, utility::ErrorState& errorState)
	{
		VkSubmitInfo compute_submit_info = {};
		compute_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		compute_submit_info.commandBufferCount = 1;
		compute_submit_info.pCommandBuffers = &mComputeCommandBuffer;

		// Signal the compute ready semaphore when we have finished running the compute shader,
		// only then the vertex input stage may be executed - graphics waits for compute to be finished
		SemaphoreWaitInfo wait_info = { mSemaphore, graphicsStageFlags };
		mRenderService->pushFrameRenderingDependency(wait_info);
		compute_submit_info.signalSemaphoreCount = 1;
		compute_submit_info.pSignalSemaphores = &mSemaphore;

		return computeInternal(numInvocations, compute_submit_info, errorState);
	}


	bool ComputeInstance::compute(uint numInvocations, utility::ErrorState& errorState)
	{
		VkSubmitInfo compute_submit_info = {};
		compute_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		compute_submit_info.commandBufferCount = 1;
		compute_submit_info.pCommandBuffers = &mComputeCommandBuffer;

		if (!computeInternal(numInvocations, compute_submit_info, errorState))
		{
			errorState.fail("Failed to run compute shader");
			return false;
		}

		// Wait for the command buffer to complete execution
		vkWaitForFences(mRenderService->getDevice(), 1, &mFence, VK_TRUE, UINT64_MAX);

		return true;
	}


	bool ComputeInstance::computeInternal(uint numInvocations, const VkSubmitInfo& submitInfo, utility::ErrorState& errorState)
	{
		// Wait for the command buffer to complete execution
		vkWaitForFences(mRenderService->getDevice(), 1, &mFence, VK_TRUE, UINT64_MAX);

		// Reset command buffer
		if (!errorState.check(vkResetCommandBuffer(mComputeCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) == VK_SUCCESS, "Failed to reset compute command buffer"))
			return false;

		// Begin command buffer
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		begin_info.pNext = nullptr;

		if (!errorState.check(vkBeginCommandBuffer(mComputeCommandBuffer, &begin_info) == VK_SUCCESS, "Failed to record to compute command buffer"))
			return false;

		// Fetch and bind pipeline
		RenderService::Pipeline pipeline = mRenderService->getOrCreateComputePipeline(mComputeMaterialInstance, errorState);
		vkCmdBindPipeline(mComputeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.mPipeline);

		// Acquire new / unique descriptor set
		const DescriptorSet& descriptor_set = mComputeMaterialInstance.updateCompute();

		// Bind shader descriptors
		vkCmdBindDescriptorSets(mComputeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

		// Dispatch compute work with a single group dimension
		uint group_count_x = numInvocations / getLocalWorkGroupSize().x + 1;
		vkCmdDispatch(mComputeCommandBuffer, group_count_x, 1, 1);

		// Copy
		mDispatchFinished(descriptor_set);

		// Add synchronization between compute and graphics queues if their queue indices are different
		if (mRenderService->getQueueIndex() != mRenderService->getComputeQueueIndex())
		{
			//VkBufferMemoryBarrier buffer_barrier = {}
			//vkCmdPipelineBarrier()...
		}

		// End recording compute commands
		if (!errorState.check(vkEndCommandBuffer(mComputeCommandBuffer) == VK_SUCCESS, "Failed to rebuild compute command buffer"))
			return false;

		// Submit
		// TODO: Keep in mind race conditions regarding semaphores when e.g. (1) switching windows or (2) destroying resources
		vkResetFences(mRenderService->getDevice(), 1, &mFence);
		if (!errorState.check(vkQueueSubmit(mRenderService->getComputeQueue(), 1, &submitInfo, mFence) == VK_SUCCESS, "Failed to submit compute work"))
			return false;

		// TODO: Use a semaphore to specify a dependency between the compute and graphics queue (VK_PIPELINE_STAGE_VERTEX_INPUT_BIT)
		// TODO: Release storage buffer if graphics and compute queue indices differ

		return true;
	}


	ComputeInstance::~ComputeInstance()
	{
		mRenderService->queueVulkanObjectDestructor([fence = mFence, semaphore = mSemaphore, cmd_buf = mComputeCommandBuffer](RenderService& renderService)
		{
			if (cmd_buf != VK_NULL_HANDLE)
				vkFreeCommandBuffers(renderService.getDevice(), renderService.getCommandPool(), 1, &cmd_buf);

			if (fence != VK_NULL_HANDLE)
				vkDestroyFence(renderService.getDevice(), fence, nullptr);

			if (semaphore != VK_NULL_HANDLE)
				vkDestroySemaphore(renderService.getDevice(), semaphore, nullptr);
		});
	}
} 
