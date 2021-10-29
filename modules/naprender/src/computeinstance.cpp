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

	static bool createSyncObjects(const VkDevice& device, std::vector<VkSemaphore>& outSemaphores, int numFramesInFlight, utility::ErrorState& errorState)
	{
		outSemaphores.resize(numFramesInFlight);

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		for (size_t i = 0; i < numFramesInFlight; i++)
		{
			if (!errorState.check(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &outSemaphores[i]) == VK_SUCCESS, "Failed to create sync objects"))
				return false;
		}
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
		if (!errorState.check(createSyncObjects(mRenderService->getDevice(), mSemaphores, mRenderService->getMaxFramesInFlight(), errorState), "Failed to create sync objects"))
			return false;

		// Initialize compute material instance
		if (!errorState.check(mComputeMaterialInstance.init(*mRenderService, *mComputeMaterialInstanceResource, errorState), "Failed to init compute material instannce"))
			return false;

		return true;
	}


	bool ComputeInstance::compute(uint numInvocations, VkPipelineStageFlags graphicsStageFlags, utility::ErrorState& errorState)
	{
		// Signal the compute ready semaphore when we have finished running the compute shader,
		// only then the vertex input stage may be executed - graphics waits for compute to be finished
		SemaphoreWaitInfo wait_info = { mSemaphores[mRenderService->getCurrentFrameIndex()], graphicsStageFlags };
		mRenderService->pushFrameRenderingDependency(wait_info);

		return computeInternal(numInvocations, errorState);
	}


	bool ComputeInstance::compute(uint numInvocations, utility::ErrorState& errorState)
	{
		return computeInternal(numInvocations, errorState);
	}


	bool ComputeInstance::computeInternal(uint numInvocations, utility::ErrorState& errorState)
	{
		// Fetch and bind pipeline
		RenderService::Pipeline pipeline = mRenderService->getOrCreateComputePipeline(mComputeMaterialInstance, errorState);
		vkCmdBindPipeline(mRenderService->getCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.mPipeline);

		std::vector<VkDescriptorSet> descriptor_sets;
		if (!mComputeMaterialInstance.update(descriptor_sets))
			return false;

		// Bind shader descriptors
		vkCmdBindDescriptorSets(mRenderService->getCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.mLayout, 0, descriptor_sets.size(), descriptor_sets.data(), 0, nullptr);

		// Dispatch compute work with a single group dimension
		uint group_count_x = numInvocations / getLocalWorkGroupSize().x + 1;
		vkCmdDispatch(mRenderService->getCurrentCommandBuffer(), group_count_x, 1, 1);

		return true;
	}


	ComputeInstance::~ComputeInstance()
	{
		mRenderService->queueVulkanObjectDestructor([semaphores = mSemaphores](RenderService& renderService)
		{
			for (auto& semaphore : semaphores)
			{
				if (semaphore != VK_NULL_HANDLE)
					vkDestroySemaphore(renderService.getDevice(), semaphore, nullptr);
			}
		});
	}
} 
