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

		ObjectPtr<RenderTarget> mTargetA = nullptr;							///< Property: The first render target
		ObjectPtr<RenderTarget> mTargetB = nullptr;							///< Property: The second render target

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
		 *	Renders all the layers associated with the currently active composition to the render targets
		 */
		void render();


		// Points to the composition component we want to render
		COMPONENT_INSTANCE_POINTER(mCompositionComponent, CompositionComponent, RenderCompositionComponent)
		COMPONENT_INSTANCE_POINTER(mRenderableComponent, RenderableMeshComponent, RenderCompositionComponent)
		COMPONENT_INSTANCE_POINTER(mCameraComponent, OrthoCameraComponent, RenderCompositionComponent)

	private:
		RenderTarget* mTargetA = nullptr;
		RenderTarget* mTargetB = nullptr;

		RenderService* mRenderService = nullptr;
	};
}
