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
	 * A particle mesh that is populated by the ParticleVolumeComponent.
	 * Includes an Id attribute to identify groups of points forming a quad.
	 */
	class ParticleMesh : public IMesh
	{
	public:
		int	mNumParticles = 1024;

		ParticleMesh(Core& core);

		/**
		 * Initialize this particle mesh
		 */
		virtual bool init(utility::ErrorState& errorState) override;

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


	/**
	 * Resource-part of ParticleVolumeComponent.
	 * 
	 * Component that manages a large group of particles on the GPU. Can be rendered as a mesh and manages
	 * at least two nap::ComputeComponents for updating the particle storage buffers.
	 *
	 * Internally, this component manages a vertex buffer and index buffer (in nap::ParticleMesh) that are
	 * used for rendering the particles as quads. Therefore, when the number of particles is 1 mln,
	 * the vertex buffer stores 4 mln vertices, and the index buffer 6 mln indices.
	 *
	 * Besides the target mesh, this component uses two particle storage buffers of type nap::StructGPUBuffer,
	 * each storing particles' positions, velocities and rotations. These are read from and written to by the
	 * update compute shader. Finally, the compute shader also writes the particle mesh positions to an
	 * additional vertex storage buffer. This buffer is also bound to the position vertex attribute buffer of
	 * the particle mesh to be rendered. When ready for rendering, the vertex and fragment shader are called
	 * with the updated positions.
	 *
	 * This component uses a bounce pass to update the particle buffers, because it's not allowed to read
	 * and write to the same buffer in the same frame. This is how the particle mesh is rendered:
	 *
	 * Frame 1: Read from buffer A, write to buffer B, bind buffer B to particle mesh
	 * Frame 2: Read from buffer B, write to buffer A, bind buffer A to particle mesh
	 * Frame 3: Read from buffer A, write to buffer B, bind buffer B to particle mesh
	 * etc.
	 *
	 * This component collects all compute component instances under the current entity and forwards the
	 * trigger for computation to the appropriate instance when ParticleVolumeComponentInstance::compute()
	 * is called. The bounce pass setup can be seen in the compute material instance resources inside these
	 * compute components.
	 *
	 * Particle Buffer A is randomly initialized with valid starting values on initialization using a
	 * nap::StructFillBufferPolicy.
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

		float										mSize = 0.5f;					///< Particle size
		float										mSpeed = 0.5f;					///< Clock speed
		float										mRotationVariation = 0.5f;		///< Rate of random variation in rotation speed
		float										mRotationSpeed = 0.1f;			///< Base rotation speed
		float										mDisplacement = 1.0f;			///< Rate of movement displacement
		int											mNumParticles = 1024;			///< Number of particles 
	};


	/**
	 * Instance-part of ParticleVolumeComponent.
	 * 
	 * Component that manages a large group of particles on the GPU. Can be rendered as a mesh and manages
	 * at least two nap::ComputeComponents for updating the particle storage buffers.
	 *
	 * Internally, this component manages a vertex buffer and index buffer (in nap::ParticleMesh) that are
	 * used for rendering the particles as quads. Therefore, when the number of particles is 1 mln,
	 * the vertex buffer stores 4 mln vertices, and the index buffer 6 mln indices.
	 *
	 * Besides the target mesh, this component uses two particle storage buffers of type nap::StructGPUBuffer,
	 * each storing particles' positions, velocities and rotations. These are read from and written to by the
	 * update compute shader. Finally, the compute shader also writes the particle mesh positions to an
	 * additional vertex storage buffer. This buffer is also bound to the position vertex attribute buffer of
	 * the particle mesh to be rendered. When ready for rendering, the vertex and fragment shader are called
	 * with the updated positions.
	 *
	 * This component uses a bounce pass to update the particle buffers, because it's not allowed to read
	 * and write to the same buffer in the same frame. This is how the particle mesh is rendered:
	 *
	 * Frame 1: Read from buffer A, write to buffer B, write positions in compute shader. Read from buffer B to particle mesh.
	 * Frame 2: Read from buffer B, write to buffer A, bind buffer A to particle mesh
	 * Frame 3: Read from buffer A, write to buffer B, bind buffer B to particle mesh
	 * etc.
	 *
	 * This component collects all compute component instances under the current entity and forwards the
	 * trigger for computation to the appropriate instance when ParticleVolumeComponentInstance::compute()
	 * is called. The bounce pass setup can be seen in the compute material instance resources inside these
	 * compute components.
	 *
	 * Particle Buffer A is randomly initialized with valid starting values on initialization using a
	 * nap::StructFillBufferPolicy.
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
		 * Updates the cached delta and elapsed time variables.
		 * @param deltaTime the time in seconds in between frames
		 */
		virtual void update(double deltaTime) override;

		/**
		 * Draws this component instance.
		 */
		void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

		/**
		 * Updates the particle storage buffers and particle mesh data with the current component instance.
		 */
		void compute();

		/**
		 * @return the number of particles.
		 */
		int getNumParticles() const										{ return mParticleMesh->mNumParticles; }

		float										mRotationVariation = 1.0f;
		float										mRotationSpeed = 1.0f;
		float										mDisplacement = 1.0f;
		float										mParticleSize = 1.0f;
		float										mSpeed = 1.0f;

	private:
		RenderService*								mRenderService = nullptr;
		std::unique_ptr<ParticleMesh>				mParticleMesh;

		double										mDeltaTime = 0.0;
		double										mElapsedTime = 0.0;

		std::vector<ComputeComponentInstance*>		mComputeInstances;					// Compute instances found in the entity
		ComputeComponentInstance*					mCurrentComputeInstance = nullptr;	// The current compute instance
		uint										mComputeInstanceIndex = 0;			// Current compute instance index
 	};
}
