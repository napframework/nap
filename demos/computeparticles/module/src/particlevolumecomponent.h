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
	class ParticleVolumeComponentInstance;
	class ParticleMesh;

	/**
	 * Structure describing the runtime state of a spawned particle
	 */
	struct Particle
	{
		Particle() {};
		glm::vec4		mPosition;					///< Current position
		glm::vec4		mVelocity;					///< Current velocity
		glm::vec4		mRotation;
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
		ComputeMaterialInstanceResource mComputeMaterial;	///< Compute material instance resource

		glm::vec3			mPosition;						///< Particle spawn position
		float				mRotation = 0.0f;				///< Start rotation
		float				mRotationVariation = 0.0f;		///< Amount of deviation from initial rotation
		float				mRotationSpeed = 0.0f;			///< How fast the particle rotates around it's axis
		float				mSize = 0.5f;					///< Default size of a particle
		float				mSpread;						///< Amount of velocity spread in x / z axis
		glm::vec3			mVelocity;						///< Initial velocity
		float				mVelocityVariation;				///< Deviation from initial velocity
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

		void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

		bool compute(utility::ErrorState& errorState);

		float mVelocityTimeScale = 5.0f;
		float mVelocityVariationScale = 1.25f;
		float mRotationSpeed = 1.0f;
		float mParticleSize = 1.0f;

	private:
		RenderService*						mRenderService = nullptr;

		UniformVec4ArrayInstance*			mPositionStorageUniform = nullptr;
		UniformVec4ArrayInstance*			mVelocityStorageUniform = nullptr;
		UniformVec4ArrayInstance*			mRotationStorageUniform = nullptr;
		//UniformVec4ArrayInstance*			mVertexStorageUniform = nullptr;

		UniformIntInstance*					mParticleCountUniform = nullptr;
		UniformFloatInstance*				mDeltaTimeUniform = nullptr;
		UniformFloatInstance*				mElapsedTimeUniform = nullptr;
		UniformFloatInstance*				mVelocityTimeScaleUniform = nullptr;
		UniformFloatInstance*				mVelocityVariationScaleUniform = nullptr;
		UniformFloatInstance*				mRotationSpeedUniform = nullptr;
		UniformFloatInstance*				mParticleSizeUniform = nullptr;

		std::vector<Particle>				mParticles;
		double								mElapsedTime = 0.0;

		std::unique_ptr<ParticleMesh>		mParticleMesh;
		std::unique_ptr<ComputeInstance>	mComputeInstance;

		const VkBuffer* mBufferPtr = nullptr;
	};
}
