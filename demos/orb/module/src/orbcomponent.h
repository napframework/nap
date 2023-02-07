/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <renderablemeshcomponent.h>
#include <mesh.h>
#include <nap/resourceptr.h>
#include <parameternumeric.h>
#include <parametervec.h>
#include <parametercolor.h>
#include <componentptr.h>

namespace nap
{
	// Forward declares
	class OrbComponentInstance;

	/**
	 * 
	 */
	class NAPAPI OrbComponent : public RenderableMeshComponent
	{
		RTTI_ENABLE(RenderableMeshComponent)
		DECLARE_COMPONENT(OrbComponent, OrbComponentInstance)
	};


	/**
	 * 
	 */
	class NAPAPI OrbComponentInstance : public RenderableMeshComponentInstance
	{
		RTTI_ENABLE(RenderableMeshComponentInstance)
	public:
		// Default constructor
		OrbComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Checks whether a transform component is available.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @param deltaTime the time in seconds in between frames
		 */
		virtual void update(double deltaTime) override;

		/**
		 * onDraw() override
		 */
		void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

		/**
		 * Returns the resource associated with this instance
		 */
		OrbComponent& getResource();

	private:
		OrbComponent*								mResource = nullptr;
		RenderService*								mRenderService = nullptr;

		double										mDeltaTime = 0.0;
		double										mElapsedTime = 0.0;
		bool										mFirstUpdate = true;
	};
}
