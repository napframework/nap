#pragma once

#include <nap/component.h>
#include <utility/dllexport.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	
	class OSCInputComponentInstance;

	/**
	 *	Receives OSC events based on the address filter
	 */
	class NAPAPI OSCInputComponent : public Component
	{
		RTTI_ENABLE(Component)
	public:
		virtual const rtti::TypeInfo getInstanceType() const override
		{
			return RTTI_OF(OSCInputComponentInstance);
		}
	};

	//////////////////////////////////////////////////////////////////////////

	class NAPAPI OSCInputComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)

	public:
		OSCInputComponentInstance(EntityInstance& entity) : ComponentInstance(entity) { }

		/**
		* Initialize this OSC input component
		* @param resource the resource we're instantiated from
		* @param used for creating new entity instances
		* @param errorState the error object
		*/
		virtual bool init(const ObjectPtr<Component>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;
	};

}
