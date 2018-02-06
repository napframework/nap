#pragma once

#include "compositioncomponent.h"

#include <component.h>
#include <componentptr.h>
#include <rendertarget.h>
#include <componentptr.h>
#include <renderablemeshcomponent.h>
#include <orthocameracomponent.h>

namespace nap
{
	class RenderCompositionComponentInstance;
	class RenderService;

	/**
	 *	rendercompositioncomponent
	 */
	class NAPAPI RenderCompositionComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(RenderCompositionComponent, RenderCompositionComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		rtti::ObjectPtr<RenderTarget> mTargetA = nullptr;							///< Property: The first render target
		rtti::ObjectPtr<RenderTarget> mTargetB = nullptr;							///< Property: The second render target

		ComponentPtr<CompositionComponent>		mCompositionComponent = nullptr;
		ComponentPtr<RenderableMeshComponent>	mRenderableComponent  = nullptr;
		ComponentPtr<OrthoCameraComponent>		mCameraComponent = nullptr;
	};


	/**
	 * rendercompositioncomponentInstance	
	 */
	class NAPAPI RenderCompositionComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		RenderCompositionComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize rendercompositioncomponentInstance based on the rendercompositioncomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the rendercompositioncomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 *	Updates targets
		 */
		virtual void update(double deltaTime) override;

		/**
		 *	Renders all the layers associated with the currently active composition to the render targets
		 */
		void render();

		/**
		 *	@return the output texture
		 */
		nap::BaseTexture2D& getTexture();

		/**
		 *	@return the output pixmap
		 */
		nap::Pixmap& getPixmap();

		// Points to the composition component we want to render
		ComponentInstancePtr<CompositionComponent> mCompositionComponent	=	{ this, &RenderCompositionComponent::mCompositionComponent };
		ComponentInstancePtr<RenderableMeshComponent> mRenderableComponent	=	{ this, &RenderCompositionComponent::mRenderableComponent };
		ComponentInstancePtr<OrthoCameraComponent> mCameraComponent =			{ this, &RenderCompositionComponent::mCameraComponent };

	private:
		RenderTarget*	mTargetA = nullptr;
		RenderTarget*	mTargetB = nullptr;
		RenderService*	mRenderService = nullptr;
		bool			mTransferring = false;

		BaseTexture2D*	inputA = nullptr;
		BaseTexture2D*	inputB = nullptr;
		RenderTarget*	activeTarget = nullptr;
		RenderTarget*	nextTarget = nullptr;

		/**
		 *	Renders a single pass 
		 * @param inputA the base input texture
		 * @param inputB the top input texture
		 * @param target the render target to render to
		 */
		void renderPass(BaseTexture2D& inputA, BaseTexture2D& inputB, RenderTarget& target);

		// Pixmap used to transfer GPU pixel values to
		nap::Pixmap mPixmap;
	};
}
