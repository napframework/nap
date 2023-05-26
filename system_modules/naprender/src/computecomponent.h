/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "renderservice.h"
#include "materialinstance.h"
#include "shadervariabledeclarations.h"

// External Includes
#include <nap/resourceptr.h>
#include <component.h>

namespace nap
{
	// Forward Declares
	class ComputeComponentInstance;

	/**
	 * Resource part of Compute Component
	 * 
	 * The compute component represents a general-purpose computation that mutates one or more storage buffers.
	 * Internally, a ComputeComponentInstance manages a ComputeMaterialInstance and caches the desired invocation count
	 * for repeated use.
	 *
	 * ComputeComponentInstance::compute() dispatches the compute shader with the given compute command buffer.
	 * Multiple compute calls may be stacked and are implicitly synchronized by the resource usage and access types bound
	 * to the compute material's descriptor sets.
	 *
	 * This component can only be used when 'Compute' is available and enabled in the RenderService, otherwise
	 * initialization fails. To enable Compute, make sure 'Compute' is marked under the 'RequiredQueues' in the
	 * RenderServiceConfiguration.
	 */
	class NAPAPI ComputeComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(ComputeComponent, ComputeComponentInstance)
	public:
		ComputeMaterialInstanceResource		mComputeMaterialInstanceResource;	///< Property: 'ComputeMaterialInstance' The compute material instance resource
		uint								mInvocations = 1;					///< Property: 'Invocations' The number of compute shader invocations per dispatch
		bool								mEnabled = true;					///< Property: 'Enabled' if this component is enabled for computation
	};


	/**
	 * Instance part of Compute Component
	 *
	 * The compute component represents a general-purpose computation that mutates one or more storage buffers.
	 * Internally, a ComputeComponentInstance manages a ComputeMaterialInstance and can caches the desired invocation count
	 * for repeated use.
	 *
	 * ComputeComponentInstance::compute() dispatches the compute shader with the given compute command buffer.
	 * Multiple compute calls may be stacked and are implicitly synchronized by the resource usage and access types bound
	 * to the compute material's descriptor sets.
	 *
	 * This component can only be used when 'Compute' is available and enabled in the RenderService, otherwise
	 * initialization fails. To enable Compute, make sure 'Compute' is marked under the 'RequiredQueues' in the
	 * RenderServiceConfiguration.
	 */
	class NAPAPI ComputeComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		ComputeComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initializes the compute instance.
		 * @param errorState contains the error if initialization fails
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Called by the renderservice, calls onCompute() with the number of invocations set in this instance
		 * @param commandBuffer active command buffer
		 */
		void compute(VkCommandBuffer commandBuffer);

		/**
		 * Called by the renderservice, calls onCompute() with the given number of invocations
		 * @param commandBuffer active command buffer
		 * @param numInvocations the number of compute shader invocations
		 */
		void compute(VkCommandBuffer commandBuffer, uint numInvocations);

		/**
		 * Toggles whether this component is enabled.
		 * @param enable if this object should be enabled or not
		 */
		void enable(bool enable)										{ mEnabled = enable; }

		/**
		 * @return if the object is enabled for computation
		 */
		bool isEnabled() const											{ return mEnabled; }

		/**
		 * @return compute program.
		 */
		ComputeMaterialInstance& getMaterialInstance() 					{ return mComputeMaterialInstance; }

		/**
		 * @return the local workgroup size
		 */
		glm::u32vec3 getWorkGroupSize() const							{ return mComputeMaterialInstance.getMaterial().getShader().getWorkGroupSize(); }

		/**
		 * Sets the number of compute shader invocations for this instance
		 * @param numInvocations the number of compute shader invocations
		 */
		void setInvocations(uint numInvocations)						{ mInvocations = numInvocations; }

		/**
		 * Returns the number of compute shader invocations for this instance
		 */
		uint getInvocations() const										{ return mInvocations; }

	protected:

		/**
		 * Dispatches the compute shader using the given command buffer, number of invocations and the bound computematerialinstance
		 * @param commandBuffer active command buffer
		 * @param numInvocations the number of compute shader invocations
		 */
		virtual void onCompute(VkCommandBuffer commandBuffer, uint numInvocations);

		/**
		 * Inserts memory barriers based on usage properties of the bufferdata. Only does so for storage buffers
		 * which are, outside color and depth attachments in render passes, the only resources that can currently be written to.
		 * Makes the assumption that the data will be read in a subsequent render pass in the current frame.
		 */
		void insertBarriers(VkCommandBuffer commandBuffer);

		RenderService*						mRenderService = nullptr;
		ComputeMaterialInstance				mComputeMaterialInstance;
		uint								mInvocations = 1;
		bool								mEnabled = true;
	};
}
