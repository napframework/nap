/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "computecomponent.h"
#include "descriptorsetcache.h"

// External includes
#include <nap/core.h>
#include <entity.h>

// nap::ComputeComponent run time class definition 
RTTI_BEGIN_CLASS(nap::ComputeComponent)
	RTTI_PROPERTY("ComputeMaterialInstance", &nap::ComputeComponent::mComputeMaterialInstanceResource, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::ComputeComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ComputeComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS


namespace nap
{
	ComputeComponentInstance::ComputeComponentInstance(EntityInstance& entity, Component& resource) :
		ComponentInstance(entity, resource),
		mRenderService(entity.getCore()->getService<RenderService>())
	{}


	bool ComputeComponentInstance::init(utility::ErrorState& errorState)
	{
		// Get resource
		ComputeComponent* resource = getComponent<ComputeComponent>();

		// Ensure compute is enabled
		if (!errorState.check(mRenderService->isComputeAvailable(), utility::stringFormat("%s: Failed to create ComputeComponentInstance because Compute is unavailable.", mID.c_str())))
			return false;

		// Initialize compute material instance
		if (!errorState.check(mComputeMaterialInstance.init(*mRenderService, resource->mComputeMaterialInstanceResource, errorState), utility::stringFormat("%s: Failed to initialize ComputeMaterialInstance", mID.c_str())))
			return false;

		return true;
	}


	void ComputeComponentInstance::update(double deltaTime)
	{

	}


	bool ComputeComponentInstance::compute(uint numInvocations, utility::ErrorState& errorState)
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
