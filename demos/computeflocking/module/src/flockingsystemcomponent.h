/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <component.h>
#include <mesh.h>
#include <renderablemeshcomponent.h>
#include <nap/resourceptr.h>
#include <computeinstance.h>

namespace nap
{
	// Forward declares
	class FlockingSystemComponentInstance;
	class ParticleMesh;

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
		ResourcePtr<ComputeInstance> mComputeInstance;		///< Property 'ComputeInstance':

		float				mSize = 0.5f;					///< Default size of a particle
		float				mVelocity = 0.5f;				///< How fast the particles move
		float				mVelocityVariation;				///< Deviation from initial velocity
		float				mRotationSpeed = 0.0f;			///< How fast the particle rotates around it's axis
		int					mNumParticles = 1024;			///< Number of particles 
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

		float mVelocityTimeScale = 0.15f;
		float mVelocityVariationScale = 0.75f;
		float mRotationSpeed = 1.0f;
		float mParticleSize = 1.0f;

		ComputeInstance* mComputeInstance = nullptr;

	private:
		RenderService*						mRenderService = nullptr;
		std::unique_ptr<ParticleMesh>		mParticleMesh;
		double								mElapsedTime = 0.0;
	};
}
