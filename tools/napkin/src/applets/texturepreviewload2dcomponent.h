/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <component.h>
#include <orthocameracomponent.h>
#include <zoompancontroller.h>
#include <componentptr.h>
#include <renderablemeshcomponent.h>
#include <uniforminstance.h>
#include <rotatecomponent.h>
#include <inputservice.h>
#include <perspcameracomponent.h>
#include <renderservice.h>
#include <orbitcontroller.h>
#include <box.h>

namespace napkin
{
	using namespace nap;
	class TexturePreviewLoad2DComponentInstance;

	/**
	 * Binds and frames a 2D texture in the viewport
	 */
	class TexturePreviewLoad2DComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(TexturePreviewLoad2DComponent, TexturePreviewLoad2DComponentInstance)
	public:

		// Properties
		ComponentPtr<ZoomPanController> mZoomPanController;				///< Property: 'ZoomPanController' the ortho camera zoom & pan controller
		ComponentPtr<TransformComponent> mPlaneTransform;				///< Property: 'PlaneTransform' the 2D texture plane transform component
		ComponentPtr<RenderableMeshComponent> mPlaneRenderer;			///< Property: 'PlaneRenderer' the 2D texture render component
		ComponentPtr<OrthoCameraComponent> mPlaneCamera;				///< Property: 'PlaneCamera' the 2D plane orthographic camera
		ComponentPtr<RenderableMeshComponent> mMeshRenderer;			///< Property: 'MeshRenderer' the 2D texture mesh render component
		ComponentPtr<PerspCameraComponent> mMeshCamera;					///< Property: 'MeshCamera' the 2D texture mesh camera
		ComponentPtr<RotateComponent> mMeshRotate;						///< Property: 'MeshRotate' the rotate component
		ComponentPtr<TransformComponent> mMeshTransform;				///< Property: 'MeshTransform' the mesh transform component
		ComponentPtr<OrbitController> mMeshOrbit;						///< Property: 'MeshOrbit' the mesh orbit controller
		ResourcePtr<Texture2D> mFallbackTexture;						///< Property: 'FallbackTexture' the default fall-back texture
		std::vector<nap::ResourcePtr<IMesh>> mMeshes;					///< Property: 'Meshes' all assignable uv meshes
	};


	/**
	 * Binds and frames a 2D texture in the viewport
	 */
	class TexturePreviewLoad2DComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(nap::ComponentInstance)
	public:
		// Display mode
		enum class EMode : uint8
		{
			Plane	= 0,		///< Orthographic plane (zoom / pan) projection
			Mesh	= 1			///< Perspective mesh projection (sphere, cube etc..)
		};

		// Constructor
		TexturePreviewLoad2DComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)					{ }

		// Destructor
		virtual ~TexturePreviewLoad2DComponentInstance()				{ mTexture.reset(nullptr); }

		/**
		 * Initialize this component
		 * @param errorState error if initialization fails
		 * @return if it initialized successfully
		 */
		bool init(utility::ErrorState& errorState) override;

		/**
		 * Loads and binds a 2D texture.
		 * Ownership is transferred to this component
		 * @param texture to load
		 */
		void load(std::unique_ptr<Texture2D> texure);

		/**
		 * Loads a 3D mesh, ownership is transferred to this component.
		 * Note that the load fails when the mesh is incompatible with texture material,
		 * in that case the mesh is immediately destroyed.
		 * @param mesh the mesh to load and select
		 * @param error contains the error if loading fails
		 * @return if the mesh is loaded and selected
		 */
		int load(std::unique_ptr<IMesh> mesh, utility::ErrorState& error);

		/**
		 * @return if there is a custom mesh
		 */
		bool hasMeshLoaded() const								{ return mMesh != nullptr; }

		/**
		 * Scales and positions current texture to perfectly fit in the viewport.
		 */
		void frame();

		/**
		 * Reverts to fall-back texture
		 */
		void clear()											{ bind(*mTextureFallback); }

		/**
		 * Current assigned texture, either the fall-back or loaded texture
		 * @return current assigned texture
		 */
		const Texture2D& getTexture() const;

		/**
		 * @return current projection mode
		 */
		EMode getMode() const									{ return mMode; }

		/**
		 * Sets the current projection mode to use
		 * @param mode new projection mode
		 */
		void setMode(EMode mode)								{ mMode = mode; }

		/**
		 * Sets the zoom & pan plane opacity.
		 * @param opacity new plane opacity
		 */
		void setOpacity(float opacity);

		/**
		 * Returns the zoom & pan plane opacity.
		 * @return zoom & pan plane opacity
		 */
		float getOpacity() const;

		/**
		 * @return current mesh index
		 */
		int getMeshIndex() const								{ return mMeshIndex; }

		/**
		 * @return set current mesh index
		 */
		void setMeshIndex(int index);

		/**
		 * @return selected mesh bounds
		 */
		const math::Box& getBounds() const;

		/**
		 * @return current selected mesh
		 */
		const IMesh& getMesh();

		/**
		 * @return all the available meshes
		 */
		const std::vector<RenderableMesh>& getMeshes() const	{ return mMeshes; }

		/**
		 * Sets mesh rotation
		 * @param rotation speed in seconds
		 */
		void setRotation(float speed)							{ mMeshRotate->setSpeed(speed); }

		/**
		 * Gets mesh rotation
		 * @return rotation speed in seconds
		 */
		float getRotation() const								{ return mMeshRotate->getSpeed(); }

		/**
		 * Process window events
		 */
		void processWindowEvents(InputService& inputService, RenderWindow& window);

		/**
		 * Draw current 2D texture on orthographic plane or on perspective mesh
		 * @param renderService service to use
		 * @param window window to render to
		 */
		void draw(RenderService& renderService, RenderWindow& window);

		// Resolved zoom & pan controller
		ComponentInstancePtr<nap::ZoomPanController> mZoomPanController = { this, &TexturePreviewLoad2DComponent::mZoomPanController };

		// Resolved plane transform
		ComponentInstancePtr<TransformComponent> mPlaneTransform = { this, &TexturePreviewLoad2DComponent::mPlaneTransform };

		// Resolved plane renderer
		ComponentInstancePtr<RenderableMeshComponent> mPlaneRenderer = { this, &TexturePreviewLoad2DComponent::mPlaneRenderer };

		// Resolved plane camera
		ComponentInstancePtr<OrthoCameraComponent> mPlaneCamera = { this, &TexturePreviewLoad2DComponent::mPlaneCamera };

		// Resolved mesh renderer
		ComponentInstancePtr<RenderableMeshComponent> mMeshRenderer = { this, &TexturePreviewLoad2DComponent::mMeshRenderer };

		// Resolved mesh camera
		ComponentInstancePtr<PerspCameraComponent> mMeshCamera = { this, &TexturePreviewLoad2DComponent::mMeshCamera };

		// Resolved mesh rotate component
		ComponentInstancePtr<RotateComponent> mMeshRotate = { this, &TexturePreviewLoad2DComponent::mMeshRotate};

		// Resolved mesh orbit component
		ComponentInstancePtr<OrbitController> mMeshOrbit = { this, &TexturePreviewLoad2DComponent::mMeshOrbit };

		// Resolved mesh transform component
		ComponentInstancePtr<TransformComponent> mMeshTransform = { this, &TexturePreviewLoad2DComponent::mMeshTransform };

	private:

		/**
		 * Bind a 2D texture
		 * @param texture the texture to bind
		 */
		void bind(Texture2D& texture);

		std::unique_ptr<Texture2D> mTexture;
		std::unique_ptr<IMesh> mMesh;
		Texture2D* mTextureFallback = nullptr;
		Sampler2DInstance* mPlaneSampler = nullptr;
		UniformFloatInstance* mPlaneOpacity = nullptr;
		Sampler2DInstance* mMeshSampler = nullptr;
		std::vector<RenderableMesh> mMeshes;
		std::vector<math::Box> mBounds;
		math::Box mPlaneBounds;
		int mMeshIndex = 0;
		EMode mMode = EMode::Plane;
		float mSpeedReference = 0.0f;
	};
}
