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
	class TexturePreviewLoadCubeComponentInstance;

	/**
	 * Binds and frames a cubemap texture in the viewport
	 */
	class TexturePreviewLoadCubeComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(TexturePreviewLoadCubeComponent, TexturePreviewLoadCubeComponentInstance)

	public:
		// Properties
		nap::ComponentPtr<RenderSkyBoxComponent> mSkyRenderer;				///< Property: 'SkyboxRenderer' the render skybox component
		nap::ComponentPtr<TransformComponent> mSkyTransform;				///< Property: 'SkyboxTransform' the skybox transform component
		nap::ComponentPtr<OrbitController> mMeshOrbit;						///< Property: 'MeshOrbit' the cubemap camera orbit controller
		nap::ComponentPtr<PerspCameraComponent> mCameraComponent;			///< Property: 'CameraComponent' the cubemap perspective camera
		nap::ComponentPtr<RenderableMeshComponent> mMeshRenderer;			///< Property: 'MeshRenderer' the reflective render mesh component
		nap::ComponentPtr<RotateComponent> mMeshRotate;						///< Property: 'MeshRotate' the rotate component
		nap::ComponentPtr<TransformComponent> mMeshTransform;				///< Property: 'MeshTransform' the transform of the mesh
		ResourcePtr<nap::TextureCube> mFallbackTexture = nullptr;			///< Property: 'FallbackTexture' the default fall-back texture
		std::vector<nap::ResourcePtr<IMesh>> mMeshes;						///< Property: 'Meshes' all assignable reflective meshes
	};


	/**
	 * Binds and frames a cubemap texture in the viewport
	 */
	class TexturePreviewLoadCubeComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		TexturePreviewLoadCubeComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

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
		 * Loads a 3D reflective mesh, ownership is transferred to this component.
		 * Note that the load fails when the mesh is incompatible with the material,
		 * in that case the mesh is immediately destroyed.
		 * @param mesh the mesh to load and select
		 * @param error contains the error if loading fails
		 * @return assigned index of the loaded mesh, -1 if mesh can't be loaded
		 */
		int load(std::unique_ptr<IMesh> mesh, utility::ErrorState& error);

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
		void setOpacity(float opacity)											{ mSkyRenderer->setOpacity(opacity); }

		/**
		 * Returns the cubemap opacity
		 * @param opacity new texture opacity
		 */
		float getOpacity() const												{ return mSkyRenderer->getOpacity(); }

		/**
		 * Sets rotation
		 * @param rotation speed in seconds
		 */
		void setRotation(float speed)											{ mMeshRotate->setSpeed(speed); }

		/**
		 * Gets rotation
		 * @return rotation speed in seconds
		 */
		float getRotation() const												{ return mMeshRotate->getSpeed(); }

		/**
		 * @return current mesh index
		 */
		int getMeshIndex() const												{ return mMeshIndex; }

		/**
		 * @return set current mesh index
		 */
		void setMeshIndex(int index);

		/**
		 * @return selected mesh bounds
		 */
		const math::Box& getBounds() const										{ assert(mMeshIndex < mBounds.size()); return mBounds[mMeshIndex]; }

		/**
		 * @return all the available meshes
		 */
		const std::vector<RenderableMesh>& getMeshes() const					{ return mMeshes; }

		/**
		 * @return current selected mesh
		 */
		const IMesh& getMesh();

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
		ComponentInstancePtr<PerspCameraComponent> mCameraComponent = { this, &TexturePreviewLoadCubeComponent::mCameraComponent };

		// Orbit controller link
		ComponentInstancePtr<OrbitController> mMeshOrbit = { this, &TexturePreviewLoadCubeComponent::mMeshOrbit };

		// Render mesh component link
		ComponentInstancePtr<RenderableMeshComponent> mMeshRenderer = { this, &TexturePreviewLoadCubeComponent::mMeshRenderer };

		// Skybox component link
		ComponentInstancePtr<RenderSkyBoxComponent> mSkyRenderer = { this, &TexturePreviewLoadCubeComponent::mSkyRenderer };

		// Rotate component link
		ComponentInstancePtr<RotateComponent> mMeshRotate = { this, &TexturePreviewLoadCubeComponent::mMeshRotate };

		// Skybox transform link
		ComponentInstancePtr<TransformComponent> mSkyTransform = { this, &TexturePreviewLoadCubeComponent::mSkyTransform };

		// Mesh transform link
		ComponentInstancePtr<TransformComponent> mMeshTransform = { this, &TexturePreviewLoadCubeComponent::mMeshTransform };

	private:
		TextureCube* mTextureFallback = nullptr;					//< Fall-back texture (managed by resource manager)
		std::unique_ptr<nap::TextureCube> mTexture = nullptr;		//< Current active loaded Cube texture
		std::unique_ptr<nap::IMesh> mMesh = nullptr;				//< Current active loaded Cube mesh
		SamplerCubeInstance* mReflectiveCubeSampler = nullptr;		//< Reflective cube sampler
		std::vector<RenderableMesh> mMeshes;						//< All meshes to select from
		std::vector<math::Box> mBounds;								//< All mesh bounding boxes
		int mMeshIndex = 0;											//< Selected mesh index
		float mSpeedReference = 0.0f;								//< Orbit move speed
		void bind(TextureCube& texture);							//< Binds a cubemap texture
	};
}
