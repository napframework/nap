#pragma once

// External Includes
#include <component.h>
#include <componentptr.h>
#include <renderablemeshcomponent.h>

namespace nap
{
	class UpdateMaterialComponentInstance;

	/**
	 * 
	 */
	class NAPAPI UpdateMaterialComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(UpdateMaterialComponent, UpdateMaterialComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ComponentPtr<RenderableMeshComponent> mStaticMeshComponent;	///< Property: 'StaticMeshComponent' link to the static mesh component
	};


	/**
	 * UpdateMaterialComponentInstance	
	 */
	class NAPAPI UpdateMaterialComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		UpdateMaterialComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize UpdateMaterialComponentInstance based on the UpdateMaterialComponent resource
		 * @param errorState should hold the error message when initialization fails
		 * @return if the UpdateMaterialComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update UpdateMaterialComponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		// Pointer to the run time Static Mesh Component Instance, set during de-serialization
		ComponentInstancePtr<RenderableMeshComponent> mStaticMeshComponent = { this, &UpdateMaterialComponent::mStaticMeshComponent };

		float* getStaticWarmthPtr();

	private:

	};
}
