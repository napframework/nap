/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <component.h>
#include <mesh.h>
#include <renderablemeshcomponent.h>

namespace nap
{
	// Forward declares
	class ParticleEmitterComponentInstance;
	class ParticleMesh;

	/**
	 * Structure describing the runtime state of a spawned particle
	 */
	struct Particle
	{
		Particle(int id) : mID(id)	{ }
		glm::vec3		mPosition;					///< Current position
		glm::vec3		mVelocity;					///< Current velocity
		glm::vec4		mColor;						///< Current color
		float			mRotation;					///< Current rotation angle in degrees
		glm::vec3		mRotationAngle;				///< Angle to rotate around
		float			mRotationSpeed;				///< Rotation speed
		float			mSize;						///< Particle size
		float			mLifeTime;					///< How long the particle lives in seconds
		float			mTimeLeft;					///< How much life it has left in seconds
		int				mID;						///< Unique particle ID
	};


	/**
	 * Component that emits a single set of particles.
	 * Internally this component manages a particle buffer, ie: it creates, removes and updates particles
	 * This object also constructs a mesh based on the current state of the particle simulation. 
	 * The particle mesh is constructed every frame. Every particle maps to a plane that consists out
	 * of 2 triangles connected using 4 vertices and 6 vertex indices. 
	 * One emitter is rendered using a single draw call, and therefore a single material.
	 */
	class ParticleEmitterComponent : public RenderableMeshComponent
	{
		RTTI_ENABLE(RenderableMeshComponent)
		DECLARE_COMPONENT(ParticleEmitterComponent, ParticleEmitterComponentInstance)

	public:
		float				mSpawnRate = 3.0f;				///< Amount of particles to spawn every second
		float				mLifeTime = 1.5f;				///< How many seconds a particle lives
		float				mLifeTimeVariation = 0.5f;		///< Variation on life in seconds
		glm::vec3			mPosition;						///< Particle spawn position
		glm::vec3			mPositionVariation;				///< Deviation from position
		float				mRotation = 0.0f;				///< Start rotation
		float				mRotationVariation = 0.0f;		///< Amount of deviation from initial rotation
		float				mRotationSpeed = 0.0f;			///< How fast the particle rotates around it's axis
		float				mRotationSpeedVariation = 0.0f;	///< Amount of deviation from particle rotation speed
		float				mSize = 0.5f;					///< Default size of a particle
		float				mSizeVariation = 0.2f;			///< Allowed deviation from the default size
		float				mSpread;						///< Amount of velocity spread in x / z axis
		glm::vec3			mVelocity;						///< Initial velocity
		float				mVelocityVariation;				///< Deviation from initial velocity
		glm::vec4			mStartColor;					///< Particle start of life color
		glm::vec4			mEndColor;						///< Particle end of life color
	};


	/**
	 * Runtime particle emitter component
	 */
	class ParticleEmitterComponentInstance : public RenderableMeshComponentInstance
	{
		RTTI_ENABLE(RenderableMeshComponentInstance)
	public:
		// Default constructor
		ParticleEmitterComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Checks whether a transform component is available.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Updates the particles and uses the result to construct a mesh
		 * @param deltaTime the time in seconds in between frames
		 */
		virtual void update(double deltaTime) override;

	private:
		/**
		 * Creates and removes particles. 
		 * Updates particle position based on velocity and it's color
		 * @param deltaTime compute time of a single frame in seconds
		 */
		void updateParticles(double deltaTime);

		/**
		 * Constructs a mesh based on the current list of particles
		 */
		void updateMesh();

	private:
		std::vector<Particle>			mParticles;
		double							mSpawntime = 0.0;
		double							mTimeSinceLastSpawn = 0.0;
		std::unique_ptr<ParticleMesh>	mParticleMesh;
		int								mCurrentID = 0;
	};
}
