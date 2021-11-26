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

namespace nap
{
	// Forward declares
	class FlockingSystemComponentInstance;

	/**
	 * A particle mesh that is populated by the ParticleVolumeComponent
	 */
	class BoidMesh : public IMesh
	{
	public:
		int	mNumBoids = 1024;

		BoidMesh(Core& core);

		/**
		 * Initialize this particle mesh
		 */
		virtual bool init(utility::ErrorState& errorState);

		/**
		 * @return MeshInstance as created during init().
		 */
		virtual MeshInstance& getMeshInstance()	override { return *mMeshInstance; }

		/**
		 * @return MeshInstance as created during init().
		 */
		virtual const MeshInstance& getMeshInstance() const	override { return *mMeshInstance; }

	private:
		std::unique_ptr<MeshInstance> mMeshInstance = nullptr;			///< The mesh instance to construct
		nap::RenderService* mRenderService = nullptr;					///< Handle to the render service
	};

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

		ResourcePtr<ParameterVec3> mTargetParam;
		ResourcePtr<ParameterFloat> mBoidSizeParam;				///< Default size of a boid
		ResourcePtr<ParameterFloat> mViewRadiusParam;
		ResourcePtr<ParameterFloat> mAvoidRadiusParam;
		ResourcePtr<ParameterFloat> mMinSpeedParam;
		ResourcePtr<ParameterFloat> mMaxSpeedParam;
		ResourcePtr<ParameterFloat> mMaxSteerForceParam;
		ResourcePtr<ParameterFloat> mTargetWeightParam;
		ResourcePtr<ParameterFloat> mAlignmentWeightParam;
		ResourcePtr<ParameterFloat> mCohesionWeightParam;
		ResourcePtr<ParameterFloat> mSeparationWeightParam;

		int mNumBoids;											///< Number of boids
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

		bool compute(utility::ErrorState& errorState);

		glm::vec4 mTarget;
		float mBoidSize;
		float mViewRadius;
		float mAvoidRadius;
		float mMinSpeed;
		float mMaxSpeed;
		float mMaxSteerForce;
		float mTargetWeight;
		float mAlignmentWeight;
		float mCohesionWeight;
		float mSeparationWeight;
		int mNumBoids;

	private:
		RenderService* mRenderService = nullptr;
		std::unique_ptr<BoidMesh>					mBoidMesh;
		double										mElapsedTime = 0.0;

		std::vector<ComputeComponentInstance*>		mComputeInstances;					// Compute instances found in the entity
		ComputeComponentInstance*					mCurrentComputeInstance = nullptr;	// The current compute instance
		uint										mComputeInstanceIndex = 0;			// Current compute instance index
		bool										mFirstUpdate = true;				// First update flag
	};
}
