/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <computecomponent.h>
#include <renderablemeshcomponent.h>
#include <mesh.h>
#include <nap/resourceptr.h>
#include <parameternumeric.h>
#include <parametervec.h>
#include <parametercolor.h>
#include <componentptr.h>
#include <perspcameracomponent.h>

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

	public:
		/**
		 * Returns the components this component depends upon.
		 * @param components the various components this component depends upon.
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override
		{
			components.emplace_back(RTTI_OF(ComputeComponent));
		}

		ResourcePtr<ParameterRGBColorFloat> mColorParam;				///< Property: "Color"

		ComponentPtr<PerspCameraComponent> mPerspCameraComponent;		///< Property: "PerspCameraComponent" Camera that we're controlling
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
		 * Execute compute shader update associated with this component
		 */
		void compute();

		/**
		 * Returns the resource associated with this instance
		 */
		OrbComponent& getResource();

	private:
		void updateRenderUniforms();
		void updateComputeUniforms(ComputeComponentInstance* comp);

		OrbComponent*								mResource = nullptr;
		RenderService*								mRenderService = nullptr;

		double										mDeltaTime = 0.0;
		double										mElapsedTime = 0.0;

		std::vector<ComputeComponentInstance*>		mComputeInstances;
		ComputeComponentInstance*					mCurrentComputeInstance = nullptr;
		int											mComputeInstanceIndex = 0;
		bool										mFirstUpdate = true;

		ComponentInstancePtr<PerspCameraComponent>	mPerspCameraComponent = { this, &OrbComponent::mPerspCameraComponent };
	};
}
