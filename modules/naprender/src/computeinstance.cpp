/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "computeinstance.h"
#include "descriptorsetcache.h"

// External includes
#include <nap/core.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ComputeInstance)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("ComputeMaterialInstance", &nap::ComputeInstance::mComputeMaterialInstanceResource, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS


namespace nap
{
	ComputeInstance::ComputeInstance(Core& core) :
		mRenderService(core.getService<RenderService>())
	{}


	bool ComputeInstance::init(utility::ErrorState& errorState)
	{
		// Ensure compute is enabled
		if (!errorState.check(mRenderService->isComputeAvailable(), "Failed to create compute instance! Compute is unavailable."))
			return false;

		// Initialize compute material instance
		if (!errorState.check(mComputeMaterialInstance.init(*mRenderService, mComputeMaterialInstanceResource, errorState), "Failed to init compute material instannce"))
			return false;

		return true;
	}


	bool ComputeInstance::compute(uint numInvocations, utility::ErrorState& errorState)
	{
		// Fetch and bind pipeline
		RenderService::Pipeline pipeline = mRenderService->getOrCreateComputePipeline(mComputeMaterialInstance, errorState);
		vkCmdBindPipeline(mRenderService->getCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.mPipeline);

		VkDescriptorSet descriptor_set = mComputeMaterialInstance.update();

		// Bind shader descriptors
		vkCmdBindDescriptorSets(mRenderService->getCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.mLayout, 0, 1, &descriptor_set, 0, nullptr);

		// Dispatch compute work with a single group dimension
		uint group_count_x = math::ceil(numInvocations / getLocalWorkGroupSize().x);
		vkCmdDispatch(mRenderService->getCurrentCommandBuffer(), group_count_x, 1, 1);

		return true;
	}
} 
