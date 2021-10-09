/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "renderservice.h"
#include "materialinstance.h"

// External Includes
#include <nap/resourceptr.h>
#include <transformcomponent.h>
#include <rect.h>

namespace nap
{
	/**
	 * Compute Instance
	 * This is a work in progress. To-do's:
	 * - Add synchronization for when compute and graphics queue indices are different
	 * - Improve abstraction/interface
	 * - Decide how this instance should be created/used (instance, resource, component?)
	 * - Finish documentation
	 */
	class NAPAPI ComputeInstance final
	{
		friend class RenderService;
	public:
		/**
		 * Default constructor
		 */
		ComputeInstance() = delete;
		ComputeInstance(uint groupCountX, uint groupCountY, uint groupCountZ, ComputeMaterialInstanceResource& computeMaterialInstanceResource, RenderService* renderService);

		// Copy constructor
		ComputeInstance(const RenderableMesh& rhs) = delete;

		// Copy assignment operator
		ComputeInstance& operator=(const ComputeInstance& rhs) = delete;

		/**
		 * @return current material used when drawing the mesh.
		 */
		ComputeMaterialInstance& getComputeMaterialInstance();

		/**
		 * Returns the compute command buffer
		 * @return the command buffer, returns nullptr if compute is not supported or enabled
		 */
		VkCommandBuffer getComputeCommandBuffer();

		/**
		 * Rebuilds the compute command buffer
		 */
		bool rebuild(utility::ErrorState& errorState);

		/**
		 * Rebuilds the compute command buffer
		 */
		bool rebuild(std::function<void(const DescriptorSet&)> copyFunc, utility::ErrorState& errorState);

		/**
		 * Sets the workgroup counts used to dispatch compute work
		 */
		void setWorkGroups(uint groupCountX, uint groupCountY, uint groupCountZ);
	
		/**
		 * Blocking
		 */
		bool compute(utility::ErrorState& errorState);

		/**
		 * Syncs the graphics queue with this compute shader.
		 * The vertex shader input stage of the current frame will not be executed until the compute shader stage is finished.
		 * Useful when compute shaders modify resources that are read by graphics shaders.
		 */
		bool asyncCompute(utility::ErrorState& errorState);

	private:
		/**
		 *
		 */
		bool init(ComputeMaterialInstanceResource& computeMaterialInstance, utility::ErrorState& errorState);

		/**
		 *
		 */
		bool computeInternal(const VkSubmitInfo& submitInfo, utility::ErrorState& errorState);

		RenderService*				mRenderService = nullptr;
		ComputeMaterialInstance		mComputeMaterialInstance;
		VkCommandBuffer				mComputeCommandBuffer = VK_NULL_HANDLE;

		glm::u32vec3				mGroupInvocations = { 64, 1, 1 };

		VkFence						mFence;
		VkSemaphore					mComputeSemaphore;

		bool						mWorkGroupsDirty = false;
		bool						mSyncGraphics = false;
	};
}
