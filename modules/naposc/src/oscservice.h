/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "oscevent.h"

// External Includes
#include <nap/service.h>
#include <entity.h>
#include <nap/datetime.h>
#include <nap/controlthread.h>

namespace nap
{
	class OSCReceiver;
	class OSCInputComponentInstance;


	/**
	 * Main interface for processing OSC messages in NAP
	 * All osc components and receivers are registered and de-registered with this service on initialization and destruction
	 * This service consumes all received osc messages and forwards them to all registered osc components.
	 * Events are only forwarded to a component if an individual address in the filter starts with the address of an osc event.
	 * Components that don't have any filter entries are forwarded all osc events
	 * Processing is handled automatically every frame
	 */
	class NAPAPI OSCService : public Service
	{
		friend class OSCReceiver;
		friend class OSCInputComponentInstance;
		RTTI_ENABLE(Service)
	public:
		// Default Constructor
		OSCService(ServiceConfiguration* configuration);

		// Default Destructor
		virtual ~OSCService();

		/**
		* Processes all received osc events from all registered osc receivers
		* The events are forwarded to all the the registered osc components
		 * @param deltaTime time in seconds. Added for compatibility with control thread.
		 */
		void process(double deltaTime);

		/**
		 * Set the control thread where incoming OSC messages will be dispatched to.
		 * If not set, incoming OSC messages will be handled on the main thread.
		 * @param controlThread The control thread OSC messages will be dispatched to.
		 */
		void setControlThread(ControlThread& controlThread);

	protected:
		/**
		 * Registers all objects that need a specific way of construction
		 * @param factory the factory to register the object creators with
		 */
		virtual void registerObjectCreators(rtti::Factory& factory) override;

		/**
		 * Initializes the osc service
		 */
		virtual bool init(nap::utility::ErrorState& errorState) override;

		/**
		* This function is called automatically by the application loop
		* If processing on update is set to true the process method will be called.
		* @param deltaTime time in between calls in seconds
		*/
		virtual void update(double deltaTime) override;

	private:
		/**
		 * Registers an OSC receiver with the service
		 */
		void registerReceiver(OSCReceiver& receiver);

		/**
		 * Removes an OSC receiver from the service
		 */
		void removeReceiver(OSCReceiver& receiver);

		/**
		 *	Register an OSC input component with the service
		 */
		void registerInputComponent(OSCInputComponentInstance& input);

		/**
		 *	Remove an osc input component from the service
		 */
		void removeInputComponent(OSCInputComponentInstance& input);

		// All the osc receivers currently registered in the system
		std::vector<OSCReceiver*> mReceivers;

		// All the osc components currently available to the system
		std::vector<OSCInputComponentInstance*> mInputs;

		// Control thread that OSC messages will be dispatched to, when set
		ControlThread* mControlThread = nullptr;

		Slot<double> mProcessSlot = { this, &OSCService::process };
	};
}