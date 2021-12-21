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
		 * Update
		 */
		virtual void update(double deltaTime) override;

		/**
		 * Syncs the graphics queue with this compute shader.
		 * The vertex shader input stage of the current frame will not be executed until the compute shader stage is finished.
		 * Useful when compute shaders modify resources that are read by graphics shaders.
		 * @param numInvocations the number of compute shader invocations
		 * @return if the compute work was submitted to the compute queue successfully.
		 */
		virtual bool compute(uint numInvocations, utility::ErrorState& errorState);

		/**
		 * @return current material used when drawing the mesh.
		 */
		ComputeMaterialInstance& getComputeMaterialInstance() 			{ return mComputeMaterialInstance; }

		/**
		 * Returns the local workgroup size
		 */
		glm::u32vec3 getLocalWorkGroupSize() const						{ return mComputeMaterialInstance.getComputeMaterial().getShader().getLocalWorkGroupSize(); }

	protected:
		RenderService*						mRenderService = nullptr;
		ComputeMaterialInstance				mComputeMaterialInstance;
	};
}
