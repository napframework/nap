/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "computeinstance.h"
#include "descriptorsetcache.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static Functions
	//////////////////////////////////////////////////////////////////////////

	static bool createSyncObjects(const VkDevice& device, VkFence& outFence, VkSemaphore& outSemaphore, utility::ErrorState& errorState)
	{
		// Create semaphore
		VkSemaphoreCreateInfo semaphore_create_info = {};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphore_create_info.pNext = nullptr;
		semaphore_create_info.flags = 0;

		if (!errorState.check(vkCreateSemaphore(device, &semaphore_create_info, nullptr, &outSemaphore) == VK_SUCCESS, "Failed to create semaphore"))
			return false;

		// Create fence
		VkFenceCreateInfo fence_create_info = {};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.pNext = nullptr;
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (!errorState.check(vkCreateFence(device, &fence_create_info, nullptr, &outFence) == VK_SUCCESS, "Failed to create fence"))
			return false;

		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	// ComputeInstance
	//////////////////////////////////////////////////////////////////////////

	ComputeInstance::ComputeInstance(ComputeMaterialInstanceResource& computeMaterialInstanceResource, RenderService* renderService) :
		mComputeMaterialInstanceResource(&computeMaterialInstanceResource), mRenderService(renderService)
	{}


	bool ComputeInstance::init(utility::ErrorState& errorState)
	{
		// If compute is enabled, create the compute command buffer
		if (!errorState.check(mRenderService->isComputeAvailable(), "Failed to create compute instance! Compute is unavailable."))
			return false;

		// Create sync objects
		if (!errorState.check(createSyncObjects(mRenderService->getDevice(), mFence, mSemaphore, errorState), "Failed to create sync objects"))
			return false;

		// Initialize compute material instance
		if (!errorState.check(mComputeMaterialInstance.init(*mRenderService, *mComputeMaterialInstanceResource, errorState), "Failed to init compute material instannce"))
			return false;

		return true;
	}


	bool ComputeInstance::asyncCompute(uint numInvocations, VkPipelineStageFlags graphicsStageFlags, utility::ErrorState& errorState)
	{
		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;

		VkCommandBuffer compute_command_buffer = mRenderService->getComputeCommandBuffer();
		submit_info.pCommandBuffers = &compute_command_buffer;

		// Signal the compute ready semaphore when we have finished running the compute shader,
		// only then the vertex input stage may be executed - graphics waits for compute to be finished
		SemaphoreWaitInfo wait_info = { mSemaphore, graphicsStageFlags };
		mRenderService->pushFrameRenderingDependency(wait_info);
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &mSemaphore;

		return computeInternal(numInvocations, submit_info, errorState);
	}


	bool ComputeInstance::compute(uint numInvocations, utility::ErrorState& errorState)
	{
		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;

		VkCommandBuffer compute_command_buffer = mRenderService->getComputeCommandBuffer();
		submit_info.pCommandBuffers = &compute_command_buffer;

		if (!computeInternal(numInvocations, submit_info, errorState))
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

		// Begin command buffer
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		begin_info.pNext = nullptr;

		if (!errorState.check(vkBeginCommandBuffer(mRenderService->getComputeCommandBuffer(), &begin_info) == VK_SUCCESS, "Failed to record to compute command buffer"))
			return false;

		// Fetch and bind pipeline
		RenderService::Pipeline pipeline = mRenderService->getOrCreateComputePipeline(mComputeMaterialInstance, errorState);
		vkCmdBindPipeline(mRenderService->getComputeCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.mPipeline);

		// Acquire new / unique descriptor set
		const DescriptorSet& descriptor_set = mComputeMaterialInstance.updateCompute();

		// Bind shader descriptors
		vkCmdBindDescriptorSets(mRenderService->getComputeCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

		// Dispatch compute work with a single group dimension
		uint group_count_x = numInvocations / getLocalWorkGroupSize().x + 1;
		vkCmdDispatch(mRenderService->getComputeCommandBuffer(), group_count_x, 1, 1);

		// Signal any connected slots for the user to issue vkCmd commands, or store descriptorset information
		mPreEndCommandBuffer(descriptor_set);

		// End recording compute commands
		if (!errorState.check(vkEndCommandBuffer(mRenderService->getComputeCommandBuffer()) == VK_SUCCESS, "Failed to rebuild compute command buffer"))
			return false;

		// Submit command buffer to compute queue
		vkResetFences(mRenderService->getDevice(), 1, &mFence);
		if (!errorState.check(vkQueueSubmit(mRenderService->getComputeQueue(), 1, &submitInfo, mFence) == VK_SUCCESS, "Failed to submit compute work"))
			return false;

		return true;
	}


	ComputeInstance::~ComputeInstance()
	{
		mRenderService->queueVulkanObjectDestructor([fence = mFence, semaphore = mSemaphore](RenderService& renderService)
		{
			if (fence != VK_NULL_HANDLE)
				vkDestroyFence(renderService.getDevice(), fence, nullptr);

			if (semaphore != VK_NULL_HANDLE)
				vkDestroySemaphore(renderService.getDevice(), semaphore, nullptr);
		});
	}
} 
