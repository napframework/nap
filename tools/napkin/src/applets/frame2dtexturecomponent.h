/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <component.h>
#include <orthocameracomponent.h>
#include <zoompancontroller.h>
#include <componentptr.h>
#include <renderablemeshcomponent.h>
#include <uniforminstance.h>
#include <rotatecomponent.h>

namespace napkin
{
	using namespace nap;
	class Frame2DTextureComponentInstance;

	/**
	 * Binds and frames a 2D texture in the viewport
	 */
	class Frame2DTextureComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(Frame2DTextureComponent, Frame2DTextureComponentInstance)
	public:

		// Properties
		ComponentPtr<ZoomPanController> mZoomPanController;				///< Property: 'ZoomPanController' the ortho camera zoom & pan controller
		ComponentPtr<TransformComponent> mPlaneTransform;				///< Property: 'PlaneTransform' the 2D texture plane transform component
		ComponentPtr<RenderableMeshComponent> mPlaneRenderer;			///< Property: 'PlaneRenderer' the 2D texture render component
		ComponentPtr<RenderableMeshComponent> mMeshRenderer;			///< Property: 'MeshRenderer' the 2D texture mesh render component
		ComponentPtr<RotateComponent> mRotateComponent;					///< Property: 'RotateComponent' the rotate component
		ResourcePtr<Texture2D> mFallbackTexture;						///< Property: 'FallbackTexture' the default fallback texture
		std::vector<nap::ResourcePtr<IMesh>> mMeshes;					///< Property: 'Meshes' all assignable uv meshes

		// Requires texture transform
		virtual void getDependentComponents(std::vector<nap::rtti::TypeInfo>& components) const override;
	};


	/**
	 * Binds and frames a 2D texture in the viewport
	 */
	class Frame2DTextureComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(nap::ComponentInstance)
	public:

		// Display mode
		enum class EMode : uint8
		{
			Plane	= 0,		///< Orthographic plane (zoom / pan) projection
			Mesh	= 1			///< Perspective mesh projection (sphere, cube etc..)
		};

		Frame2DTextureComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize this component
		 * @param errorState error if initialization fails
		 * @return if it initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Bind a 2D texture
		 * @param texture the texture to bind
		 * @param frame if the texture is framed in the window
		 */
		void bind(Texture2D& texture);

		/**
		 * Scales and positions current texture to perfectly fit in the viewport.
		 */
		void frame();

		/**
		 * Reverts to fall-back texture
		 */
		void clear();

		/**
		 * @return current projection mode
		 */
		EMode getMode() const						{ return mMode; }

		/**
		 * Sets the current projection mode to use
		 * @param mode new projection mode
		 */
		void setMode(EMode mode)					{ mMode = mode; }

		/**
		 * Sets the texture opacity
		 * @param opacity new texture opacity
		 */
		void setOpacity(float opacity);

		/**
		 * Returns the texture opacity
		 * @param opacity new texture opacity
		 */
		float getOpacity() const;

		/**
		 * @return current mesh index
		 */
		int getMeshIndex() const					{ return mMeshIndex; }

		/**
		 * @return set current mesh index
		 */
		void setMeshIndex(int index);

		// Resolved zoom & pan controller
		ComponentInstancePtr<nap::ZoomPanController> mZoomPanController = { this, &Frame2DTextureComponent::mZoomPanController };

		// Resolved plane transform
		ComponentInstancePtr<TransformComponent> mPlaneTransform = { this, &Frame2DTextureComponent::mPlaneTransform };

		// Resolved plane renderer
		ComponentInstancePtr<RenderableMeshComponent> mPlaneRenderer = { this, &Frame2DTextureComponent::mPlaneRenderer };

		// Resolved plane renderer
		ComponentInstancePtr<RenderableMeshComponent> mMeshRenderer = { this, &Frame2DTextureComponent::mMeshRenderer };

	private:
		Texture2D* mSelectedTexture = nullptr;
		Texture2D* mTextureFallback = nullptr;
		Sampler2DInstance* mPlaneSampler = nullptr;
		UniformFloatInstance* mPlaneOpacity = nullptr;
		Sampler2DInstance* mMeshSampler = nullptr;
		UniformFloatInstance* mMeshOpacity = nullptr;
		std::vector<RenderableMesh> mMeshes;
		int mMeshIndex = 0;
		EMode mMode = EMode::Plane;
	};
}
