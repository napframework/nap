#pragma once

// Local Includes
#include "oscevent.h"

// External Includes
#include <nap/component.h>
#include <utility/dllexport.h>
#include <nap/signalslot.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	
	class OSCInputComponentInstance;
	class OSCService;

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

		// Holds the allowed OSC addresses, when empty every osc event is valid to
		// be received by this component, otherwise only the onces that specified by the filter
		std::vector<std::string>		mAddressFilter;
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
		 *	Initializes the component and copies over the osc addresses
		 */
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		// Holds a set of OSC addresses, when empty every osc event is valid to
		// be received by this component, otherwise only the ones that are present in this list
		std::vector<std::string>		mAddressFilter;

		// Connect to this signal to receive osc events
		Signal<const OSCEvent&>			messageReceived;

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
