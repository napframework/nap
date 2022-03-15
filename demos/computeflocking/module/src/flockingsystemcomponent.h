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

		uint mNumBoids;													///< Property: "NumBoids" The number of boids

		ResourcePtr<ParameterBool> mRandomColorParam;					///< Property: "RandomColorParam" Enable random colors
		ResourcePtr<ParameterFloat> mBoidSizeParam;						///< Property: "BoidSizeParam" Default size of a boid
		ResourcePtr<ParameterFloat> mFresnelScaleParam;					///< Property: "FresnelScale" Scale fresnel term
		ResourcePtr<ParameterFloat> mFresnelPowerParam;					///< Property: "FresnelPower" Apply power to fresnel term

		ResourcePtr<ParameterFloat> mViewRadiusParam;					///< Property: "ViewRadius" Radius in which a boid detects mates to follow
		ResourcePtr<ParameterFloat> mAvoidRadiusParam;					///< Property: "AvoidRadius" Radius in which a boid detect mates to avoid
		ResourcePtr<ParameterFloat> mMinSpeedParam;						///< Property: "MinSpeed" Minimum speed
		ResourcePtr<ParameterFloat> mMaxSpeedParam;						///< Property: "MaxSteerForce" Maximum speed
		ResourcePtr<ParameterFloat> mMaxSteerForceParam;				///< Property: "MaxSteerForce" Maximum steer force, also acceleration and deceleration
		ResourcePtr<ParameterFloat> mTargetWeightParam;					///< Property: "TargetWeight" Contribution of target on boid forces
		ResourcePtr<ParameterFloat> mAlignmentWeightParam;				///< Property: "AlignmentWeight" Contribution of aligment vector to boid forces
		ResourcePtr<ParameterFloat> mCohesionWeightParam;				///< Property: "CohesionWeight" Contribution of cohesion vector on boid movement
		ResourcePtr<ParameterFloat> mSeparationWeightParam;				///< Property: "SeparationWeight" Contibution of separation vector on boid movement
		ResourcePtr<ParameterFloat> mBoundsRadiusParam;					///< Property: "BoundsRadius" Radius of bounding sphere

		ResourcePtr<ParameterVec3> mLightPositionParam;					///< Property: "LightPosition" Light position
		ResourcePtr<ParameterFloat> mLightIntensityParam;				///< Property: "LightIntensity" Light intensity
		ResourcePtr<ParameterRGBColorFloat> mDiffuseColorParam;			///< Property: "DiffuseColorParam" Diffuse color
		ResourcePtr<ParameterRGBColorFloat> mDiffuseColorExParam;		///< Property: "DiffuseColorExParam" Secondary diffuse color when in proximity of mates
		ResourcePtr<ParameterRGBColorFloat> mLightColorParam;			///< Property: "LightColorParam" Light color
		ResourcePtr<ParameterRGBColorFloat> mHaloColorParam;			///< Property: "HaloColorParam" Halo color
		ResourcePtr<ParameterRGBColorFloat> mSpecularColorParam;		///< Property: "SpecularColor" Specular color
		ResourcePtr<ParameterFloat> mShininessParam;					///< Property: "Shininess" Shininess
		ResourcePtr<ParameterFloat> mAmbientIntensityParam;				///< Property: "AmbientIntensity" Ambient intensity
		ResourcePtr<ParameterFloat> mDiffuseIntensityParam;				///< Property: "DiffuseIntensity" Diffuse intensity
		ResourcePtr<ParameterFloat> mSpecularIntensityParam;			///< Property: "SpecularIntensity" Specular intensity
		ResourcePtr<ParameterFloat> mMateColorRateParam;				///< Property: "MateColor" Maximum rate of mates for blending boid diffuse colors

		ComponentPtr<PerspCameraComponent> mPerspCameraComponent;		///< Property: "PerspCameraComponent" Camera that we're controlling
		ComponentPtr<TransformComponent> mTargetTransformComponent;		///< Property: "TargetTransformComponent" Camera that we're controlling
	};


	/**
	 * Runtime 
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
		FlockingSystemComponent& getResource() const;

		/**
		 * Returns the number of boids in this flocking system
		 */
		uint getNumBoids() const;

	private:
		void updateRenderMaterial();
		void updateComputeMaterial(ComputeComponentInstance* comp);

		FlockingSystemComponent*					mResource = nullptr;
		RenderService*								mRenderService = nullptr;

		uint										mNumBoids = 1000;
		double										mDeltaTime = 0.0;
		double										mElapsedTime = 0.0;

		std::vector<ComputeComponentInstance*>		mComputeInstances;					//< Compute instances found in the entity
		ComputeComponentInstance*					mCurrentComputeInstance = nullptr;	//< The current compute instance
		uint										mComputeInstanceIndex = 0;			//< Current compute instance index

		ComponentInstancePtr<PerspCameraComponent>	mPerspCameraComponent = { this, &FlockingSystemComponent::mPerspCameraComponent };
		ComponentInstancePtr<TransformComponent>	mTargetTransformComponent = { this, &FlockingSystemComponent::mTargetTransformComponent };
	};
}
