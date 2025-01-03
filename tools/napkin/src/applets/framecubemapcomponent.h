/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <component.h>
#include <componentptr.h>
#include <orbitcontroller.h>
#include <renderablemeshcomponent.h>
#include <renderskyboxcomponent.h>
#include <rotatecomponent.h>
#include <mesh.h>

namespace napkin
{
	using namespace nap;
	class FrameCubemapComponentInstance;

	/**
	 * Binds and frames a cubemap texture in the viewport
	 */
	class FrameCubemapComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(FrameCubemapComponent, FrameCubemapComponentInstance)

	public:
		// Properties
		nap::ComponentPtr<RenderSkyBoxComponent> mSkyBoxComponent;			///< Property: 'SkyboxComponent' the render skybox component
		nap::ComponentPtr<OrbitController> mOrbitController;				///< Property: 'OrbitController' the cubemap camera orbit controller
		nap::ComponentPtr<RenderableMeshComponent> mRenderMeshComponent;	///< Property: 'MeshComponent' the reflective render mesh component
		nap::ComponentPtr<RotateComponent> mRotateComponent;				///< Property: 'RotateComponent' the rotate component
		ResourcePtr<nap::TextureCube> mFallbackTexture = nullptr;			///< Property: 'FallbackTexture' the default fallback texture
		std::vector<nap::ResourcePtr<IMesh>> mMeshes;						///< Property: 'Meshes' all assignable reflective meshes
	};


	/**
	 * Binds and frames a cubemap texture in the viewport
	 */
	class FrameCubemapComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		FrameCubemapComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize this compnent
		 * @param errorState the error message when initialization fails
		 * @return if the component initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Bind a Cubemap texture
		 * @param texture the texture to bind
		 * @param frame if the texture is framed in the window
		 */
		void bind(TextureCube& texture);

		/**
		 * Scales and positions current texture to perfectly fit in the viewport.
		 */
		void frame();

		/**
		 * Reverts to fall-back texture
		 */
		void clear();

		/**
		 * Sets the cubemap opacity
		 * @param opacity new texture opacity
		 */
		void setOpacity(float opacity)											{ mSkyboxComponent->setOpacity(opacity); }

		/**
		 * Returns the cubemap opacity
		 * @param opacity new texture opacity
		 */
		float getOpacity() const												{ return mSkyboxComponent->getOpacity(); }

		/**
		 * Sets rotation
		 * @param rotation speed in seconds
		 */
		void setRotation(float speed)											{ mRotateComponent->setSpeed(speed); }

		/**
		 * Gets rotation
		 * @return rotation speed in seconds
		 */
		float getRotation(float speed) const									{ mRotateComponent->getSpeed(); }

		/**
		 * @return current mesh index
		 */
		int getMeshIndex() const												{ return mMeshIndex; }

		/**
		 * @return set current mesh index
		 */
		void setMeshIndex(int index);

		/**
		 * @return all the available meshes
		 */
		const std::vector<RenderableMesh>& getMeshes() const					{ return mMeshes; }

		// Orbit controller link
		ComponentInstancePtr<OrbitController> mOrbitController = { this, &FrameCubemapComponent::mOrbitController };

		// Render mesh component link
		ComponentInstancePtr<RenderableMeshComponent> mRenderMeshComponent = { this, &FrameCubemapComponent::mRenderMeshComponent };

		// Skybox component link
		ComponentInstancePtr<RenderSkyBoxComponent> mSkyboxComponent = { this, &FrameCubemapComponent::mSkyBoxComponent };

		// Rotate component link
		ComponentInstancePtr<RotateComponent> mRotateComponent = { this, &FrameCubemapComponent::mRotateComponent };

		// Fallback texture
		TextureCube* mTextureFallback = nullptr;

		// Reflective cube sampler
		SamplerCubeInstance* mReflectiveCubeSampler = nullptr;

		//< All meshes to select from
		std::vector<RenderableMesh> mMeshes;

		// Mesh idx
		int mMeshIndex = 0;
	};
}
