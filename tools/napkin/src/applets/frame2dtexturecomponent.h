#pragma once

#include <component.h>
#include <orthocameracomponent.h>
#include <zoompancontroller.h>
#include <componentptr.h>
#include <renderablemeshcomponent.h>

namespace napkin
{
	class Frame2DTextureComponentInstance;

	/**
	 * Binds and frames a 2D texture in the viewport
	 */
	class Frame2DTextureComponent : public nap::Component
	{
		RTTI_ENABLE(nap::Component)
		DECLARE_COMPONENT(Frame2DTextureComponent, Frame2DTextureComponentInstance)

	public:

		// Properties
		nap::ComponentPtr<nap::ZoomPanController> mZoomPanController = nullptr;			///< Property: 'ZoomPanController' the ortho camera zoom & pan controller
		nap::ResourcePtr<nap::Texture2D> mFallbackTexture = nullptr;					///< Property: 'FallbackTexture' the default fallback texture


		// Requires texture transform
		virtual void getDependentComponents(std::vector<nap::rtti::TypeInfo>& components) const override;
	};


	/**
	 * Binds and frames a 2D texture in the viewport
	 */
	class Frame2DTextureComponentInstance : public nap::ComponentInstance
	{
		RTTI_ENABLE(nap::ComponentInstance)
	public:
		Frame2DTextureComponentInstance(nap::EntityInstance& entity, nap::Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize this component
		 * @param errorState error if initialization fails
		 * @return if it initialized successfully
		 */
		virtual bool init(nap::utility::ErrorState& errorState) override;

		/**
		 * update appletcomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * Bind a 2D texture
		 * @param texture the texture to bind
		 * @param frame if the texture is framed in the window
		 */
		void bind(nap::Texture2D& texture);

		/**
		 * Scales and positions current texture to perfectly fit in the viewport.
		 */
		void frame();

		/**
		 * Reverts to fall-back texture
		 */
		void clear();

		// Resolved zoom & pan controller
		nap::ComponentInstancePtr<nap::ZoomPanController> mZoomPanController = { this, &Frame2DTextureComponent::mZoomPanController };

	private:
		nap::Texture2D* mSelectedTexture = nullptr;
		nap::Texture2D* mTextureFallback = nullptr;
		nap::Sampler2DInstance* mSampler = nullptr;
		nap::TransformComponentInstance* mTextureTransform = nullptr;
		nap::RenderableMeshComponentInstance* mTextureRenderer = nullptr;
	};
}
