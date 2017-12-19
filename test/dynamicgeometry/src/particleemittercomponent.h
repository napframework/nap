#pragma once

#include "component.h"
#include "mesh.h"

namespace nap
{
	class TransformComponent;
	class ParticleEmitterComponentInstance;
	
	/**
	 * Structure describing the runtime state of a spawned particle
	 */
	struct Particle
	{
		glm::vec3		mPosition;
		glm::vec3		mVelocity;
		glm::vec4		mColor;
		float			mRotation;
		float			mRotationSpeed;
		float			mSize;
		float			mLifeTime;
		float			mTimeLeft;
	};

	/**
	 * Component for emitting a single set of particle. One emitter maps to a single drawcall, and therefore a single material.
	 */
	class ParticleEmitterComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(ParticleEmitterComponent, ParticleEmitterComponentInstance)

		/**
		 * 
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override { components.push_back(RTTI_OF(TransformComponent)); }

	public:
		ObjectPtr<Mesh>		mMesh;
		float				mSpawnRate = 3.0f;
		float				mLifeTime = 1.5f;
		float				mLifeTimeVariation = 0.5f;
		glm::vec3			mPosition;
		glm::vec3			mPositionVariation;
		float				mRotation = 0.0f;
		float				mRotationVariation = 0.0f;
		float				mRotationSpeed = 0.0f;
		float				mRotationSpeedVariation = 0.0f;
		float				mSize = 0.5f;
		float				mSizeVariation = 0.2f;
		float				mSpread;
		float				mVelocity;
		float				mVelocityVariation;
		glm::vec4			mStartColor;
		glm::vec4			mEndColor;
	};

	/**
	 * Instance object for Particle emitters.
	 */
	class ParticleEmitterComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		// Default constructor
		ParticleEmitterComponentInstance(EntityInstance& entity, Component& resource);

		virtual ~ParticleEmitterComponentInstance() override;

		/**
		 * Checks whether a transform component is available.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		virtual void update(double deltaTime) override;

	private:
		void updateParticles(double deltaTime);
		void updateMesh();

	private:
		std::vector<Particle>	mParticles;
		double					mTimeSinceLastSpawn = 1000.0;
	};
}
