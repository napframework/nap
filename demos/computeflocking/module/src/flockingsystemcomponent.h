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
	class FlockingSystemComponentInstance;

	/**
	 * Component that emits a single set of particles.
	 * Internally this component manages a particle buffer, ie: it creates, removes and updates particles
	 * This object also constructs a mesh based on the current state of the particle simulation. 
	 * The particle mesh is constructed every frame. Every particle maps to a plane that consists out
	 * of 2 triangles connected using 4 vertices and 6 vertex indices. 
	 * One emitter is rendered using a single draw call, and therefore a single material.
	 */
	class NAPAPI FlockingSystemComponent : public RenderableMeshComponent
	{
		RTTI_ENABLE(RenderableMeshComponent)
		DECLARE_COMPONENT(FlockingSystemComponent, FlockingSystemComponentInstance)

	public:
		/**
		 * Returns the components this component depends upon.
		 * @param components the various components this component depends upon.
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override
		{
			components.emplace_back(RTTI_OF(ComputeComponent));
		}

		int mNumBoids;													///< Property: "NumBoids" The number of boids

		ResourcePtr<ParameterBool> mRandomColorParam;					///< Property: "RandomColorParam" Enable random colors
		ResourcePtr<ParameterFloat> mBoidSizeParam;						///< Property: "BoidSizeParam" Default size of a boid
		ResourcePtr<ParameterFloat> mFresnelScaleParam;					///< Property: "" 
		ResourcePtr<ParameterFloat> mFresnelPowerParam;					///< Property: "" 

		ResourcePtr<ParameterFloat> mViewRadiusParam;					///< Property: "" 
		ResourcePtr<ParameterFloat> mAvoidRadiusParam;					///< Property: "" 
		ResourcePtr<ParameterFloat> mMinSpeedParam;						///< Property: "" 
		ResourcePtr<ParameterFloat> mMaxSpeedParam;						///< Property: "" 
		ResourcePtr<ParameterFloat> mMaxSteerForceParam;				///< Property: "" 
		ResourcePtr<ParameterFloat> mTargetWeightParam;					///< Property: "" 
		ResourcePtr<ParameterFloat> mAlignmentWeightParam;				///< Property: "" 
		ResourcePtr<ParameterFloat> mCohesionWeightParam;				///< Property: "" 
		ResourcePtr<ParameterFloat> mSeparationWeightParam;				///< Property: "" 
		ResourcePtr<ParameterFloat> mBoundsExtentParam;					///< Property: "" 

		ResourcePtr<ParameterVec3> mLightPositionParam;					///< Property: "" 
		ResourcePtr<ParameterFloat> mLightIntensityParam;				///< Property: "" 
		ResourcePtr<ParameterRGBColorFloat> mDiffuseColorParam;			///< Property: "" 
		ResourcePtr<ParameterRGBColorFloat> mLightColorParam;			///< Property: "" 
		ResourcePtr<ParameterRGBColorFloat> mSpecularColorParam;		///< Property: "" 
		ResourcePtr<ParameterFloat> mShininessParam;					///< Property: "" 
		ResourcePtr<ParameterFloat> mAmbientIntensityParam;				///< Property: "" 
		ResourcePtr<ParameterFloat> mDiffuseIntensityParam;				///< Property: "" 
		ResourcePtr<ParameterFloat> mSpecularIntensityParam;			///< Property: "" 

		ComponentPtr<PerspCameraComponent> mPerspCameraComponent;		///< Property: "PerspCameraComponent" Camera that we're controlling
		ComponentPtr<TransformComponent> mTargetTransformComponent;		///< Property: "TargetTransformComponent" Camera that we're controlling
	};


	/**
	 * Runtime particle emitter component
	 */
	class NAPAPI FlockingSystemComponentInstance : public RenderableMeshComponentInstance
	{
		RTTI_ENABLE(RenderableMeshComponentInstance)
	public:
		// Default constructor
		FlockingSystemComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Checks whether a transform component is available.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Updates the particles and uses the result to construct a mesh
		 * @param deltaTime the time in seconds in between frames
		 */
		virtual void update(double deltaTime) override;

		void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

		void compute();

		glm::vec4 mTarget;
		int mNumBoids;

	private:
		FlockingSystemComponent*					mResource = nullptr;
		RenderService*								mRenderService = nullptr;
		double										mElapsedTime = 0.0;

		std::vector<ComputeComponentInstance*>		mComputeInstances;					// Compute instances found in the entity
		ComputeComponentInstance*					mCurrentComputeInstance = nullptr;	// The current compute instance
		uint										mComputeInstanceIndex = 0;			// Current compute instance index
		bool										mFirstUpdate = true;				// First update flag

		ComponentInstancePtr<PerspCameraComponent>	mPerspCameraComponent = { this, &FlockingSystemComponent::mPerspCameraComponent };
		ComponentInstancePtr<TransformComponent>	mTargetTransformComponent = { this, &FlockingSystemComponent::mTargetTransformComponent };
	};
}
