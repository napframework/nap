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
#include <inputservice.h>
#include <renderservice.h>

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
		nap::ComponentPtr<PerspCameraComponent> mCameraComponent;			///< Property: 'CameraComponent' the cubemap perspective camera
		nap::ComponentPtr<RenderableMeshComponent> mRenderMeshComponent;	///< Property: 'MeshComponent' the reflective render mesh component
		nap::ComponentPtr<RotateComponent> mRotateComponent;				///< Property: 'RotateComponent' the rotate component
		ResourcePtr<nap::TextureCube> mFallbackTexture = nullptr;			///< Property: 'FallbackTexture' the default fall-back texture
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

		virtual ~FrameCubemapComponentInstance();

		/**
		 * Initialize this component
		 * @param errorState the error message when initialization fails
		 * @return if the component initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Loads and binds a cubemap texture.
		 * Ownership is transferred to this component
		 * @param texture to load
		 */
		void load(std::unique_ptr<TextureCube> texure);

		/**
		 * Loads and selects a 3D reflective mesh.
		 * Ownership is transferred to this component.
		 * Note that the load fails when the mesh is incompatible with the material,
		 * in that case the mesh is immediately destroyed.
		 * @param mesh the mesh to load and select
		 * @param error contains the error if loading fails
		 * @return if the mesh is loaded and selected
		 */
		bool load(std::unique_ptr<IMesh> mesh, utility::ErrorState& error);

		/**
		 * Scales and positions current texture to perfectly fit in the viewport.
		 */
		void frame();

		/**
		 * Reverts to fall-back texture
		 */
		void clear();

		/**
		 * Current assigned texture, either the fall-back or loaded texture
		 * @return current assigned texture
		 */
		const TextureCube& getTexture() const;

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
		float getRotation() const												{ return mRotateComponent->getSpeed(); }

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

		/**
		 * @return if there is a custom mesh loaded and selectable
		 */
		bool hasMeshLoaded() const;

		/**
		 * Process received window events (mouse etc.)
		 * @param inputService the service collecting input events
		 * @param window the render window
		 */
		void processWindowEvents(nap::InputService& inputService, nap::RenderWindow& window);

		/**
		 * Draws current cubemap and reflective mesh
		 * @param renderService service to use
		 * @param window window to render to
		 */
		void draw(RenderService& renderService, RenderWindow& window);

		// Perspective camera component link
		ComponentInstancePtr<PerspCameraComponent> mCameraComponent = { this, &FrameCubemapComponent::mCameraComponent };

		// Orbit controller link
		ComponentInstancePtr<OrbitController> mOrbitController = { this, &FrameCubemapComponent::mOrbitController };

		// Render mesh component link
		ComponentInstancePtr<RenderableMeshComponent> mRenderMeshComponent = { this, &FrameCubemapComponent::mRenderMeshComponent };

		// Skybox component link
		ComponentInstancePtr<RenderSkyBoxComponent> mSkyboxComponent = { this, &FrameCubemapComponent::mSkyBoxComponent };

		// Rotate component link
		ComponentInstancePtr<RotateComponent> mRotateComponent = { this, &FrameCubemapComponent::mRotateComponent };

	private:
		TextureCube* mTextureFallback = nullptr;					//< Fall-back texture (managed by resource manager)
		std::unique_ptr<nap::TextureCube> mTexture = nullptr;		//< Current active loaded Cube texture
		std::unique_ptr<nap::IMesh> mMesh = nullptr;				//< Current active loaded Cube mesh
		SamplerCubeInstance* mReflectiveCubeSampler = nullptr;		//< Reflective cube sampler
		std::vector<RenderableMesh> mMeshes;						//< All meshes to select from
		int mMeshIndex = 0;											//< Selected mesh index
		void bind(TextureCube& texture);							//< Binds a cubemap texture
	};
}
