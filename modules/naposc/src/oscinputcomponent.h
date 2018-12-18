#pragma once

// Local Includes
#include "oscevent.h"

// External Includes
#include <component.h>
#include <utility/dllexport.h>
#include <nap/signalslot.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	
	class OSCInputComponentInstance;
	class OSCService;

	/**
	 * Receives osc events based on the address filter
	 * The address filter is a list of strings that represent individual osc addresses
	 * When the osc service forwards an event it checks if one of the names in the address filter starts with the address of the osc event. 
	 * If that's the case the instance of this component receives an osc event. 
	 * When the address filter is empty all events are forwarded.
	 */
	class NAPAPI OSCInputComponent : public Component
	{
		RTTI_ENABLE(Component)
	public:
		virtual const rtti::TypeInfo getInstanceType() const override
		{
			return RTTI_OF(OSCInputComponentInstance);
		}

		std::vector<std::string>		mAddressFilter;		///< Property: 'Addresses' list of OSC addresses this component is allowed to receive, when empty all events are forwarded
	};

	//////////////////////////////////////////////////////////////////////////

	/**
	 * Instance of the OSC input component
	 * This component will forward any received osc messages to listening components
	 * Register to the @messageReceived event to receive osc events that match the address pattern
	 */
	class NAPAPI OSCInputComponentInstance : public ComponentInstance
	{
		friend class OSCService;
		RTTI_ENABLE(ComponentInstance)
	public:
		// Default constructor
		OSCInputComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }
		
		// Unregister with service
		virtual ~OSCInputComponentInstance();

		/**
		 * Initializes the component and copies over the osc addresses
		 * @param errorState contains the error when the component could not be initialized
		 * @return if initialization succeeded
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		std::vector<std::string>		mAddressFilter;			///< List of available osc addresses, when empty all osc events are forwarded
		Signal<const OSCEvent&>			messageReceived;		///< Triggered when the component receives an osc message
        
        Signal<const OSCEvent&>* getMessageReceived() { return &messageReceived; }

	protected:
		/**
		 * This is triggered by the service when a new osc message is received
		 * @param oscEvent the osc event that will be forwarded by this component
		 */
		void trigger(const nap::OSCEvent& oscEvent);

	private:
		OSCService* mService = nullptr;					// OSC Service set when initialized
	};

}
