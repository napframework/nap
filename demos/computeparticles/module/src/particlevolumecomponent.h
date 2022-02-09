/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <component.h>
#include <mesh.h>
#include <renderablemeshcomponent.h>
#include <componentptr.h>
#include <computecomponent.h>

namespace nap
{
	// Forward declares
	class ParticleVolumeComponentInstance;
	class Core;

	/**
	 * A particle mesh that is populated by the ParticleVolumeComponent
	 */
	class ParticleMesh : public IMesh
	{
	public:
		int	mNumParticles = 1024;

		ParticleMesh(Core& core);

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
	class NAPAPI ParticleVolumeComponent : public RenderableMeshComponent
	{
		RTTI_ENABLE(RenderableMeshComponent)
		DECLARE_COMPONENT(ParticleVolumeComponent, ParticleVolumeComponentInstance)

	public:
		/**
		 * Returns the components this component depends upon.
		 * @param components the various components this component depends upon.
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override
		{
			components.emplace_back(RTTI_OF(ComputeComponent));
		}

		float				mSize = 0.5f;					///< Default size of a particle
		float				mVelocity = 0.5f;				///< How fast the particles move
		float				mVelocityVariation;				///< Deviation from initial velocity
		float				mRotationSpeed = 0.0f;			///< How fast the particle rotates around it's axis
		int					mNumParticles = 1024;			///< Number of particles 
	};


	/**
	 * Runtime particle emitter component
	 */
	class NAPAPI ParticleVolumeComponentInstance : public RenderableMeshComponentInstance
	{
		RTTI_ENABLE(RenderableMeshComponentInstance)
	public:
		// Default constructor
		ParticleVolumeComponentInstance(EntityInstance& entity, Component& resource);

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
		 * Draws this component instance
		 */
		void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

		/**
		 * Computes a new component update 
		 */
		void compute();

		/**
		 * @return the number of particles
		 */
		int getNumParticles() const					{ return mParticleMesh->mNumParticles; }

		float mVelocityTimeScale = 0.15f;
		float mVelocityVariationScale = 0.75f;
		float mRotationSpeed = 1.0f;
		float mParticleSize = 1.0f;

	private:
		RenderService*								mRenderService = nullptr;
		std::unique_ptr<ParticleMesh>				mParticleMesh;
		double										mElapsedTime = 0.0;

		std::vector<ComputeComponentInstance*>		mComputeInstances;					// Compute instances found in the entity
		ComputeComponentInstance*					mCurrentComputeInstance = nullptr;	// The current compute instance
		uint										mComputeInstanceIndex = 0;			// Current compute instance index
		bool										mFirstUpdate = true;				// First update flag
	};
}
