#pragma once

#include "pointlightcomponent.h"
#include "selectledmeshcomponent.h"

#include <nap/component.h>
#include <nap/componentptr.h>
#include <transformcomponent.h>
#include <renderablemeshcomponent.h>

namespace nap
{
	class SyncLedMaterialComponentInstance;

	/**
	 *	updatematerialcomponent
	 */
	class SyncLedMaterialComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(SyncLedMaterialComponent, SyncLedMaterialComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ComponentPtr<PointlightComponent>		mPointLightComponent = nullptr;
		ComponentPtr<TransformComponent>		mPointLightTransform = nullptr;
		ComponentPtr<TransformComponent>		mCameraTransform = nullptr;
		ComponentPtr<SelectLedMeshComponent>	mSelectMeshComponent = nullptr;
	};


	/**
	 * updatematerialcomponentInstance	
	 */
	class SyncLedMaterialComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		SyncLedMaterialComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize updatematerialcomponentInstance based on the updatematerialcomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the updatematerialcomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update updatematerialcomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		ComponentInstancePtr<PointlightComponent> mPointlightComponent =	{ this, &SyncLedMaterialComponent::mPointLightComponent };
		ComponentInstancePtr<TransformComponent> mPointLightTransform =		{ this, &SyncLedMaterialComponent::mPointLightTransform };
		ComponentInstancePtr<TransformComponent> mCameraTransform =			{ this, &SyncLedMaterialComponent::mCameraTransform };
		ComponentInstancePtr<SelectLedMeshComponent> mLedSelectComponent =	{ this, &SyncLedMaterialComponent::mSelectMeshComponent };

	};
}
