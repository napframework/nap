#pragma once

#include <component.h>

namespace nap
{
	class $fileinputname$Instance;

	/**
	 *	$fileinputname$
	 */
	class NAPAPI $fileinputname$ : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT($fileinputname$, $fileinputname$Instance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};


	/**
	 * $fileinputname$Instance	
	 */
	class NAPAPI $fileinputname$Instance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		$fileinputname$Instance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize $fileinputname$Instance based on the $fileinputname$ resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the $fileinputname$Instance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update $fileinputname$Instance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;
	};
}
