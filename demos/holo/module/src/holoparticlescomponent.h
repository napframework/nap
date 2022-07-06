/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <component.h>
#include <mesh.h>
#include <rendercomponent.h>
#include <componentptr.h>
#include <uniforminstance.h>
#include <materialinstance.h>
#include <renderablemesh.h>
#include <parameternumeric.h>
#include <transformcomponent.h>

namespace nap
{
	// Forward declares
	class HoloParticlesComponentInstance;
	class MaterialInstance;
	class MaterialInstanceResource;
	class RenderableMesh;
	class Core;

	/**
	 * Resource-part of HoloParticlesComponent.
	 *
	 * A custom component for the holo demo that renders a bunch of floating particle meshes using instancing. Relies
	 * heavily on the included shader program. All positions and rotations are computed using a simplex noise
	 * function in the vertex shader. The number of particles is limited by the maximum size of an uniform buffer object.
	 */
	class NAPAPI HoloParticlesComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
		DECLARE_COMPONENT(HoloParticlesComponent, HoloParticlesComponentInstance)

	public:
		rtti::ObjectPtr<IMesh>		mMesh;							///< Property: 'Mesh' mesh to render.
		MaterialInstanceResource	mMaterialInstanceResource;		///< Property: 'MaterialInstance' the material used to render the mesh.

		ResourcePtr<ParameterFloat>	mSizeParameter;					///< Property: 'SizeParameter' the particle size [0.0, x].
		ResourcePtr<ParameterFloat>	mSpeedParameter;				///< Property: 'SpeedParameter' the clock speed of the particle update timer [0.0, x].
		ResourcePtr<ParameterFloat>	mRainbowBlendParameter;			///< Property: 'RainbowBlendParameter' the rainbow blend value [0.0, 1.0].
		int							mNumParticles = 1024;			///< Property: 'NumParticles' Number of particles.

		ComponentPtr<TransformComponent> mCameraTransformComponent;	///< Property: 'CameraTransformComponent' transform that controls the camera.
	};


	/**
	 * Instance-part of HoloParticlesComponent.
	 *
	 * A custom component for the holo demo that renders a bunch of floating particle meshes using instancing. Relies
	 * heavily on the included shader program. All positions and rotations are computed using a simplex noise
	 * function in the vertex shader. The number of particles is limited by the maximum size of an uniform buffer object.
	 */
	class NAPAPI HoloParticlesComponentInstance : public RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)
	public:
		// Default constructor
		HoloParticlesComponentInstance(EntityInstance& entity, Component& resource);

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
		 * @return the number of particles.
		 */
		int getNumParticles() const										{ return mResource->mNumParticles; }

	protected:
		/**
		 * Renders the model from the ModelResource, using the material on the ModelResource.
		 */
		virtual void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	private:
		RenderService*								mRenderService = nullptr;
		HoloParticlesComponent*						mResource = nullptr;

		MaterialInstance							mMaterialInstance;						///< The MaterialInstance as created from the resource
		RenderableMesh								mRenderableMesh;						///< Renderable mesh

		math::Rect									mClipRect;								///< Clipping rectangle for this instance, in pixel coordinates

		nap::UniformMat4Instance*					mProjectionUniform = nullptr;			///< Projection matrix uniform slot
		nap::UniformMat4Instance*					mViewUniform = nullptr;					///< View matrix uniform slot
		nap::UniformMat4Instance*					mModelUniform = nullptr;				///< Model matrix uniform slot

		float										mSize = 0.2f;
		float										mSpeed = 0.1f;
		float										mRainbowBlend = 0.0f;

		int											mCount = 1;
		double										mDeltaTime = 0.0;
		double										mElapsedTime = 0.0;

		const float									mSizeLimitation = 0.125f;

		ComponentInstancePtr<TransformComponent>	mCameraTransformComponent = { this, &HoloParticlesComponent::mCameraTransformComponent };
 	};
}
