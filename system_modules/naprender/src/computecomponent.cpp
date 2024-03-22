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
	RTTI_PROPERTY("Enabled",					&nap::ComputeComponent::mEnabled,							nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ComputeMaterialInstance",	&nap::ComputeComponent::mComputeMaterialInstanceResource,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Invocations",				&nap::ComputeComponent::mInvocations,						nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::ComputeComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ComputeComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static functions
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Sets a pipeline barrier
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
	// ComputeComponentInstance
	//////////////////////////////////////////////////////////////////////////

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

		mInvocations = resource->mInvocations;

		return true;
	}


	void ComputeComponentInstance::compute(VkCommandBuffer commandBuffer)
	{
		if (!isEnabled())
			return;

		onCompute(commandBuffer, mInvocations);
	}


	void ComputeComponentInstance::compute(VkCommandBuffer commandBuffer, uint numInvocations)
	{
		if (!isEnabled())
			return;

		onCompute(commandBuffer, numInvocations);
	}


	void ComputeComponentInstance::onCompute(VkCommandBuffer commandBuffer, uint numInvocations)
	{
		// Fetch and bind pipeline
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreateComputePipeline(mComputeMaterialInstance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.mPipeline);

		const DescriptorSet& descriptor_set = mComputeMaterialInstance.update();

		// Bind shader descriptors
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

		// Dispatch compute work with a single group dimension
		uint group_count_x = (numInvocations / getWorkGroupSize().x) + 1;
		vkCmdDispatch(commandBuffer, group_count_x, 1, 1);

		// Insert memory barriers if required
		insertBarriers(commandBuffer);
	}


	void ComputeComponentInstance::insertBarriers(VkCommandBuffer commandBuffer)
	{
		for (const auto& binding : mComputeMaterialInstance.getBufferBindings())
		{
			// Fetch buffer data
			const auto& buffer_data = binding->getBuffer().getBufferData();

			// Check if the resource is marked to be written to (in any stage)
			if (buffer_data.mUsage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
			{
				// We must set a memory barrier to prevent access to the resource before it is finished being written to
				VkAccessFlags dst_access = 0;
				VkPipelineStageFlags dst_stage = 0;

				// The resource may be consumed as a vertex attribute buffer
				if (buffer_data.mUsage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
				{
					dst_access |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
					dst_stage |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
				}

				// The resource may be consumed as an index buffer
				if (buffer_data.mUsage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
				{
					dst_access |= VK_ACCESS_INDEX_READ_BIT;
					dst_stage |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
				}

				// We assume the resource is read in the vertex and fragment shader
				dst_access = (dst_access == 0) ? VK_ACCESS_SHADER_READ_BIT : dst_access;
				dst_stage = (dst_stage == 0) ? VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT : dst_stage;

				// Insert a memory barrier for this resource
				memoryBarrier(commandBuffer, buffer_data.mBuffer, VK_ACCESS_SHADER_WRITE_BIT, dst_access, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, dst_stage);
			}
		}
	}
} 
