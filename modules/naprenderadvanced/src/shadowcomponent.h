#pragma once

// External includes
#include <component.h>
#include <componentptr.h>

namespace nap
{
	class ShadowComponentInstance;

	/**
	 *	ShadowComponent
	 */
	class NAPAPI ShadowComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(ShadowComponent, ShadowComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};


	/**
	 * ShadowComponentInstance	
	 */
	class NAPAPI ShadowComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		ShadowComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize ShadowComponentInstance based on the ShadowComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the ShadowComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update ShadowComponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

	private:
		ShadowComponent* mResource = nullptr;
	};
}
