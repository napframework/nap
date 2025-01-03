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
		ComponentPtr<RenderableMeshComponent> mPlaneRenderer;			///< Property: 'PlaneMeshComponent' the 2D texture render component
		ResourcePtr<Texture2D> mFallbackTexture;						///< Property: 'FallbackTexture' the default fallback texture

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
		 * Sets the texture opacity
		 * @param opacity new texture opacity
		 */
		void setOpacity(float opacity)				{ assert(mOpacity != nullptr); mOpacity->setValue(opacity); }

		/**
		 * Returns the texture opacity
		 * @param opacity new texture opacity
		 */
		float getOpacity() const					{ assert(mOpacity != nullptr); return mOpacity->getValue(); }

		// Resolved zoom & pan controller
		ComponentInstancePtr<nap::ZoomPanController> mZoomPanController = { this, &Frame2DTextureComponent::mZoomPanController };

		// Resolved plane transform
		ComponentInstancePtr<TransformComponent> mPlaneTransform = { this, &Frame2DTextureComponent::mPlaneTransform };

		// Resolved plane renderer
		ComponentInstancePtr<RenderableMeshComponent> mPlaneRenderer = { this, &Frame2DTextureComponent::mPlaneRenderer };

	private:
		Texture2D* mSelectedTexture = nullptr;
		Texture2D* mTextureFallback = nullptr;
		Sampler2DInstance* mSampler = nullptr;
		UniformFloatInstance* mOpacity = nullptr;
	};
}
