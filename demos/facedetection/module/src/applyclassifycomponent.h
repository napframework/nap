#pragma once

#include <component.h>
#include <componentptr.h>
#include <cvclassifycomponent.h>
#include <rendertotexturecomponent.h>

namespace nap
{
	class ApplyClassifyComponentInstance;

	/**
	 * Pushes detected objects to material on update.
	 */
	class NAPAPI ApplyClassifyComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(ApplyClassifyComponent, ApplyClassifyComponentInstance)

	public:
		nap::ComponentPtr<CVClassifyComponent> mClassifyComponent = nullptr;		///< Property: 'ClassifyComponent' the OpenCV classification component.
		nap::ComponentPtr<RenderToTextureComponent> mRenderComponent = nullptr;		///< Property: 'RenderComponent'
	};


	/**
	 * applydetectioncomponentInstance	
	 */
	class NAPAPI ApplyClassifyComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		ApplyClassifyComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize applydetectioncomponentInstance based on the applydetectioncomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the applydetectioncomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update applydetectioncomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		nap::ComponentInstancePtr<CVClassifyComponent> mClassifyComponent		= { this, &ApplyClassifyComponent::mClassifyComponent };
		nap::ComponentInstancePtr<RenderToTextureComponent> mRenderComponent	= { this, &ApplyClassifyComponent::mRenderComponent };
	};
}
