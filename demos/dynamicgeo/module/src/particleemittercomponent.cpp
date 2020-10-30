/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "particleemittercomponent.h"

// External Includes
#include <entity.h>
#include <rect.h>
#include <glm/gtx/transform.hpp>
#include <mathutils.h>
#include <nap/core.h>
#include <renderservice.h>
#include <renderglobals.h>

RTTI_BEGIN_CLASS(nap::ParticleEmitterComponent)
	RTTI_PROPERTY("SpawnRate",				&nap::ParticleEmitterComponent::mSpawnRate,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("LifeTime",				&nap::ParticleEmitterComponent::mLifeTime,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Position",				&nap::ParticleEmitterComponent::mPosition,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("PositionVariation",		&nap::ParticleEmitterComponent::mPositionVariation,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("LifeTimeVariation",		&nap::ParticleEmitterComponent::mLifeTimeVariation,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Size",					&nap::ParticleEmitterComponent::mSize,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SizeVariation",			&nap::ParticleEmitterComponent::mSizeVariation,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Rotation",				&nap::ParticleEmitterComponent::mRotation,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RotationVariation",		&nap::ParticleEmitterComponent::mRotationVariation,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RotationSpeed",			&nap::ParticleEmitterComponent::mRotationSpeed,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RotationSpeedVariation", &nap::ParticleEmitterComponent::mRotationSpeedVariation,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Spread",					&nap::ParticleEmitterComponent::mSpread,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Velocity",				&nap::ParticleEmitterComponent::mVelocity,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("VelocityVariation",		&nap::ParticleEmitterComponent::mVelocityVariation,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("StartColor",				&nap::ParticleEmitterComponent::mStartColor,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("EndColor",				&nap::ParticleEmitterComponent::mEndColor,					nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParticleEmitterComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	// Default normalized plane uv's
	static glm::vec3 plane_uvs[] =
	{
		{ 0.0f,	0.0f,	0.0f },
		{ 1.0f,	0.0f,	0.0f },
		{ 0.0f,	1.0f,	0.0f },
		{ 1.0f,	1.0f,	0.0f },
	};


	/**
	 * Randomizes a value based on a deviation from that value
	 * @param baseValue the default value
	 * @param variation the amount that value is allowed to deviate from it's original value
	 */
	template<typename T>
	static T particleRand(T baseValue, T variation)
	{
		return math::random<T>(baseValue - variation, baseValue + variation);
	}


	//////////////////////////////////////////////////////////////////////////

	/**
	 * A particle mesh that is populated by the ParticleEmitterComponent
	 */
	class ParticleMesh : public IMesh
	{
	public:
		ParticleMesh(Core& core) : mRenderService(core.getService<RenderService>())
		{ }

		bool init(utility::ErrorState& errorState)
		{
			assert(mRenderService != nullptr);
			mMeshInstance = std::make_unique<MeshInstance>(*mRenderService);

			// Because the mesh is populated dynamically we set the initial amount of vertices to be 0
			mMeshInstance->setNumVertices(0);
			mMeshInstance->setUsage(EMeshDataUsage::DynamicWrite);
			mMeshInstance->reserveVertices(1000);
			mMeshInstance->setDrawMode(EDrawMode::Triangles);
			mMeshInstance->setCullMode(ECullMode::None);

			// Create the necessary attributes
			Vec3VertexAttribute& position_attribute = mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::position);
			Vec3VertexAttribute& uv_attribute = mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::getUVName(0));
			Vec4VertexAttribute& color_attribute = mMeshInstance->getOrCreateAttribute<glm::vec4>(vertexid::getColorName(0));
			FloatVertexAttribute& id_attribute = mMeshInstance->getOrCreateAttribute<float>("pid");

			MeshShape& shape = mMeshInstance->createShape();

			// Reserve CPU memory for all the particle geometry necessary to create
			// We want to draw the mesh as a set of triangles, 2 triangles per particle
			shape.reserveIndices(1000);

			// Initialize our instance
			return mMeshInstance->init(errorState);
		}

		/**
		* @return MeshInstance as created during init().
		*/
		virtual MeshInstance& getMeshInstance()	override				{ return *mMeshInstance; }

		/**
		* @return MeshInstance as created during init().
		*/
		virtual const MeshInstance& getMeshInstance() const	override	{ return *mMeshInstance; }

	private:
		std::unique_ptr<MeshInstance> mMeshInstance = nullptr;			///< The mesh instance to construct
		nap::RenderService* mRenderService = nullptr;					///< Handle to the render service
	};

	//////////////////////////////////////////////////////////////////////////

	ParticleEmitterComponentInstance::ParticleEmitterComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableMeshComponentInstance(entity, resource),
		mParticleMesh(std::make_unique<ParticleMesh>(*entity.getCore()))
	{ }


	bool ParticleEmitterComponentInstance::init(utility::ErrorState& errorState)
	{
		// initialize base class
		if (!RenderableMeshComponentInstance::init(errorState))
			return false;

		// Initialize particle mesh
		if (!errorState.check(mParticleMesh->init(errorState), "Unable to create particle mesh"))
			return false;

		// Bind the particle mesh to the material and create a VAO
		RenderableMesh renderableMesh = createRenderableMesh(*mParticleMesh, errorState);
		if (!renderableMesh.isValid())
			return false;

		// Set the particle mesh to be used when drawing
		setMesh(renderableMesh);

		// Calculate the amount of time it takes within a second to spawn a new particle
		mSpawntime = static_cast<double>(1.0f / getComponent<ParticleEmitterComponent>()->mSpawnRate);

		// This ensures a particle is spawned immediately
		mTimeSinceLastSpawn = mSpawntime;

		return true;
	}


	void ParticleEmitterComponentInstance::updateParticles(double deltaTime)
	{
		// Calculate the amount of time that passes between now and last particle spawn
		ParticleEmitterComponent* component = getComponent<ParticleEmitterComponent>();
		mTimeSinceLastSpawn += deltaTime;

		// If enough time passed we want to spawn at least 1 particle
		// If a lot of time passed we want to spawn more
		if (mTimeSinceLastSpawn >= mSpawntime)
		{
			// Calculate number of particles to spawn
			int pcount = static_cast<int>(std::round(mTimeSinceLastSpawn / mSpawntime));
			assert(pcount != 0);

			// Create a new particle and add to our list of particles
			for (int i = 0; i < pcount; i++)
			{
				Particle particle(mCurrentID++);
				particle.mPosition = particleRand(component->mPosition, component->mPositionVariation);
				particle.mRotation = particleRand(component->mRotation, component->mRotationVariation);
				particle.mRotationAngle = glm::normalize(particleRand<glm::vec3>({ 0.0,0.0,0.0 }, { 1.0,1.0,1.0 }));
				particle.mRotationSpeed = particleRand(component->mRotationSpeed, component->mRotationSpeedVariation);
				particle.mSize = particleRand(component->mSize, component->mSizeVariation);
				particle.mLifeTime = particleRand(component->mLifeTime, component->mLifeTimeVariation);
				float spread = particleRand(0.0f, component->mSpread);
				float velocity_x = particleRand(component->mVelocity.x, component->mVelocityVariation);
				float velocity_y = particleRand(component->mVelocity.y, component->mVelocityVariation);
				float velocity_z = particleRand(component->mVelocity.z, component->mVelocityVariation);
				particle.mVelocity = glm::vec3(spread, 1.0f, spread);
				particle.mVelocity = glm::normalize(particle.mVelocity);
				particle.mVelocity *= glm::vec3(velocity_x, velocity_y, velocity_z);
				particle.mTimeLeft = particle.mLifeTime;

				mParticles.emplace_back(particle);
				mCurrentID = mCurrentID % math::max<int>();
			}
			// Reset spawn time
			mTimeSinceLastSpawn = 0.0f;
		}

		// Iterate over all particles and kill the ones that ran out of time
		for (int i = mParticles.size() - 1; i >= 0; --i)
		{
			Particle& particle = mParticles[i];
			particle.mTimeLeft -= deltaTime;
			if (particle.mTimeLeft < 0.0)
			{
				mParticles.erase(mParticles.begin() + i);
				continue;
			}

			// Otherwise update their position and rotation
			particle.mPosition += particle.mVelocity * (float)deltaTime;
			particle.mRotation += particle.mRotationSpeed * (float)deltaTime;

			// Update color based on time over life
			float life_scale = 1.0f - (particle.mTimeLeft / component->mLifeTime);
			particle.mColor = component->mStartColor + (component->mEndColor - component->mStartColor) * life_scale;
			particle.mColor.a = math::bell(life_scale, 2.0f);
		}
	}


	void ParticleEmitterComponentInstance::updateMesh()
	{
 		MeshInstance& mesh_instance = mParticleMesh->getMeshInstance();

		// Get the total number of vertices based on particle count
		// When the mesh is updated on the gpu it knows the amount of data to upload
		int num_vertices = mParticles.size() * 4;
		mesh_instance.setNumVertices(num_vertices);

		// Get the attributes we want to modify
		Vec3VertexAttribute& position_attribute = mesh_instance.getOrCreateAttribute<glm::vec3>(vertexid::position);
		Vec3VertexAttribute& uv_attribute		= mesh_instance.getOrCreateAttribute<glm::vec3>(vertexid::getUVName(0));
		Vec4VertexAttribute& color_attribute	= mesh_instance.getOrCreateAttribute<glm::vec4>(vertexid::getColorName(0));
		FloatVertexAttribute& id_attribute		= mesh_instance.getOrCreateAttribute<float>("pid");

		// Clear all of 'm
		position_attribute.clear();
		uv_attribute.clear();
		color_attribute.clear();
		id_attribute.clear();

		MeshShape& shape = mesh_instance.getShape(0);
		shape.clearIndices();

		// Build the mesh based on the amount of particles
		unsigned int cur_num_vertices = 0;
		for (Particle& particle : mParticles)
		{
			// Create the position vertices
			float halfSize = particle.mSize * 0.5f;
			math::Rect rect(-halfSize, -halfSize, particle.mSize, particle.mSize);

			// Get rectangle positions
			glm::vec3 positions[] =
			{
				{rect.getMin().x,	rect.getMin().y, 0.0f },
				{rect.getMax().x,	rect.getMin().y, 0.0f },
				{rect.getMin().x,	rect.getMax().y, 0.0f },
				{rect.getMax().x,	rect.getMax().y, 0.0f },
			};

			// Apply particle translation / rotation
			glm::mat4x4 translation = glm::translate(particle.mPosition);
			glm::mat4x4 rotation = glm::rotate(particle.mRotation, particle.mRotationAngle);
			for (glm::vec3& pos : positions)
			{
				glm::vec4 world_pos = translation * rotation * glm::vec4(pos, 1.0f);
				pos = glm::vec3(world_pos.x, world_pos.y, world_pos.z);
			}

			// Push particle id
			float ids[] =
			{
				float(particle.mID),
				float(particle.mID),
				float(particle.mID),
				float(particle.mID)
			};

			// Indices for 2 triangles, 1 plane
			unsigned int indices[] =
			{
				cur_num_vertices + 0,
				cur_num_vertices + 1,
				cur_num_vertices + 3,
				cur_num_vertices + 0,
				cur_num_vertices + 3,
				cur_num_vertices + 2
			};

			// Push particle color
			glm::vec4 colors[] =
			{
				particle.mColor,
				particle.mColor,
				particle.mColor,
				particle.mColor
			};

			// Add data
			position_attribute.addData(positions, 4);
			uv_attribute.addData(plane_uvs, 4);
			color_attribute.addData(colors, 4);
			id_attribute.addData(ids, 4);
			shape.addIndices(indices, 6);
			cur_num_vertices += 4;
		}

		// push
		utility::ErrorState error_state;
		mesh_instance.update(error_state);
	}

	void ParticleEmitterComponentInstance::update(double deltaTime)
	{
		updateParticles(deltaTime);
		updateMesh();

		RenderableMeshComponentInstance::update(deltaTime);
	}
}