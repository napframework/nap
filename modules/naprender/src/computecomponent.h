/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "renderservice.h"
#include "materialinstance.h"

// External Includes
#include <nap/resourceptr.h>
#include <component.h>

namespace nap
{
	// Forward Declares
	class ComputeComponentInstance;

	/**
	 * Compute Component
	 * This is a work in progress. To-do's:
	 * - Improve abstraction/interface
	 * - Decide how this instance should be created/used (instance, resource, component?)
	 * - Finish documentation
	 */
	class NAPAPI ComputeComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(ComputeComponent, ComputeComponentInstance)
	public:
		ComputeMaterialInstanceResource		mComputeMaterialInstanceResource;	///< Property 'ComputeMaterialInstance' The compute material instance resource
		uint								mInvocations = 1;
	};

	/**
	 * Compute Component Instance
	 */
	class NAPAPI ComputeComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		/**
		 * Constructor
		 */
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
		 * @return current material used when drawing the mesh.
		 */
		ComputeMaterialInstance& getComputeMaterialInstance() 			{ return mComputeMaterialInstance; }

		/**
		 * Returns the local workgroup size
		 */
		glm::u32vec3 getLocalWorkGroupSize() const						{ return mComputeMaterialInstance.getComputeMaterial().getShader().getLocalWorkGroupSize(); }

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
		 * Inserts memory barriers 
		 */
		void insertBarriers(VkCommandBuffer commandBuffer, const std::vector<BufferData>& resources);

		RenderService*						mRenderService = nullptr;
		ComputeMaterialInstance				mComputeMaterialInstance;
		uint								mInvocations = 1;
	};
}
