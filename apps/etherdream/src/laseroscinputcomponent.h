#pragma once

// Local Includes
#include "oscinputcomponent.h"

// External Includes
#include <component.h>
#include <utility/dllexport.h>
#include <nap/signalslot.h>

namespace nap
{
	class LaserOSCInputComponentInstance;

	/**
	 *	Receives OSC events based on the address filter
	 */
	class LaserOSCInputComponent : public OSCInputComponent
	{
		RTTI_ENABLE(OSCInputComponent)
	public:
		virtual const rtti::TypeInfo getInstanceType() const override
		{
			return RTTI_OF(LaserOSCInputComponentInstance);
		}

		// property: the LaserID for which the OSC addresses should be filtered
		int							mLaserID;
	};

	//////////////////////////////////////////////////////////////////////////

	/**
	 * Instance of the OSC input component
	 * This component will forward any received osc messages to listening components
	 * Register to the @messageReceived event to receive osc events that match the address pattern
	 */
	class LaserOSCInputComponentInstance : public OSCInputComponentInstance
	{
		RTTI_ENABLE(OSCInputComponentInstance)
	public:
		// Default constructor
		LaserOSCInputComponentInstance(EntityInstance& entity, Component& resource) : OSCInputComponentInstance(entity, resource) { }

		/**
		 *	Initializes the component and copies over the osc addresses
		 */
		virtual bool init(utility::ErrorState& errorState) override;
	};

}
