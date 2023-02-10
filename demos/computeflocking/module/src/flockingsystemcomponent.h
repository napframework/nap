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
	 * Resource-part of FlockingSystemComponent.
	 *
	 * Component that manages a large group of boids on the GPU. Can be rendered as a mesh and manages
	 * at least two nap::ComputeComponents for updating the boid storage buffers.
	 *
	 * This component generates a nap::StructGPUBuffer comprising of boid data. The most important properties
	 * of a boid are its position, velocity (direction and magnitude) and orientation (a quaternion). The
	 * layout and contents of the boid buffers described in the material in JSON match those defined in `flock.comp`.
	 * 
	 * The output buffer of a particular compute shader dispatch is bound to a storage buffer input in the vertex
	 * shader of the flocking system. This component uses instanced rendering to draw the same boid model thousands
	 * of times. With instanced rendering, we can use the built-in variable `gl_InstanceIndex` to identify what boid
	 * is being rendered. This index is used as a key to fetch the appropriate boid information from the storage
	 * buffer. We calculate each boid's world transformation from the position and orientation from the buffer. 
	 * 
	 * This component uses a bounce pass to update the boid buffers, because it's not allowed to read
	 * and write to the same buffer in the same frame. This is how the boids are rendered.
	 *
	 * Frame 1: Read from buffer A, write to buffer B, bind buffer B to vertex shader
	 * Frame 2: Read from buffer B, write to buffer A, bind buffer A to vertex shader
	 * Frame 3: Read from buffer A, write to buffer B, bind buffer B to vertex shader
	 * etc.
	 *
	 * This component collects all compute component instances under the current entity and forwards the
	 * trigger for computation to the appropriate instance when FlockingSystemComponentInstance::compute()
	 * is called. The bounce pass setup can be seen in the compute material instance resources inside these
	 * compute components.
	 *
	 * Boid buffer A is randomly initialized with valid starting values on initialization using a
	 * nap::StructFillBufferPolicy.
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

		uint						mNumBoids = 1000;					///< Property: "NumBoids" The number of boids

		ResourcePtr<ParameterBool>	mRandomColorParam;					///< Property: "RandomColorParam" Enable random colors
		ResourcePtr<ParameterFloat> mBoidSizeParam;						///< Property: "BoidSizeParam" Default size of a boid
		ResourcePtr<ParameterFloat> mFresnelScaleParam;					///< Property: "FresnelScale" Scale fresnel term
		ResourcePtr<ParameterFloat> mFresnelPowerParam;					///< Property: "FresnelPower" Apply power to fresnel term

		ResourcePtr<ParameterFloat> mViewRadiusParam;					///< Property: "ViewRadius" Radius in which a boid detects mates to follow
		ResourcePtr<ParameterFloat> mAvoidRadiusParam;					///< Property: "AvoidRadius" Radius in which a boid detect mates to avoid
		ResourcePtr<ParameterFloat> mMinSpeedParam;						///< Property: "MinSpeed" Minimum speed
		ResourcePtr<ParameterFloat> mMaxSpeedParam;						///< Property: "MaxSpeed" Maximum speed
		ResourcePtr<ParameterFloat> mTargetWeightParam;					///< Property: "TargetWeight" Contribution of target on boid forces
		ResourcePtr<ParameterFloat> mAlignmentWeightParam;				///< Property: "AlignmentWeight" Contribution of aligment vector to boid forces
		ResourcePtr<ParameterFloat> mCohesionWeightParam;				///< Property: "CohesionWeight" Contribution of cohesion vector on boid movement
		ResourcePtr<ParameterFloat> mSeparationWeightParam;				///< Property: "SeparationWeight" Contibution of separation vector on boid movement
		ResourcePtr<ParameterFloat> mBoundsRadiusParam;					///< Property: "BoundsRadius" Radius of bounding sphere

		ResourcePtr<ParameterVec3>	mLightPositionParam;				///< Property: "LightPosition" Light position
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

		std::vector<ComponentPtr<TransformComponent>> mTargetTransforms; ///< Property: "TargetTransforms" List of boid targets
	};


	/**
	 * Instance-part of FlockingSystemComponent.
	 *
	 * Component that manages a large group of boids on the GPU. Can be rendered as a mesh and manages
	 * at least two nap::ComputeComponents for updating the boid storage buffers.
	 *
	 * This component generates a nap::StructGPUBuffer comprising of boid data. The most important properties
	 * of a boid are its position, velocity (direction and magnitude) and orientation (a quaternion). The
	 * layout and contents of the boid buffers described in the material in JSON match those defined in `flock.comp`.
	 *
	 * The output buffer of a particular compute shader dispatch is bound to a storage buffer input in the vertex
	 * shader of the flocking system. This component uses instanced rendering to draw the same boid model thousands
	 * of times. With instanced rendering, we can use the built-in variable `gl_InstanceIndex` to identify what boid
	 * is being rendered. This index is used as a key to fetch the appropriate boid information from the storage
	 * buffer. We calculate each boid's world transformation from the position and orientation from the buffer.
	 *
	 * This component uses a bounce pass to update the boid buffers, because it's not allowed to read
	 * and write to the same buffer in the same frame. This is how the boids are rendered.
	 *
	 * Frame 1: Read from buffer A, write to buffer B, bind buffer B to vertex shader
	 * Frame 2: Read from buffer B, write to buffer A, bind buffer A to vertex shader
	 * Frame 3: Read from buffer A, write to buffer B, bind buffer B to vertex shader
	 * etc.
	 *
	 * This component collects all compute component instances under the current entity and forwards the
	 * trigger for computation to the appropriate instance when FlockingSystemComponentInstance::compute()
	 * is called. The bounce pass setup can be seen in the compute material instance resources inside these
	 * compute components.
	 *
	 * Boid buffer A is randomly initialized with valid starting values on initialization using a
	 * nap::StructFillBufferPolicy.
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
		 * Updates the particles and uses the result to construct a mesh.
		 * @param deltaTime the time in seconds in between frames
		 */
		virtual void update(double deltaTime) override;

		/**
		 * onDraw() override
		 */
		void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

		/**
		 * Updates the boid storage buffer with the current component instance.
		 */
		void compute();

		/**
		 * Returns the resource associated with this instance.
		 */
		FlockingSystemComponent& getResource() const;

		/**
		 * Returns the number of boids in this flocking system.
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
		int											mComputeInstanceIndex = 0;			//< Current compute instance index

		std::vector<ComponentInstancePtr<TransformComponent>> mTargetTransforms = initComponentInstancePtr(this, &FlockingSystemComponent::mTargetTransforms);
	};
}
