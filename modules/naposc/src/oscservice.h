#pragma once
// Local Includes
#include "oscevent.h"

// External Includes
#include <nap/service.h>
#include <entity.h>
#include <utility/datetimeutils.h>

namespace nap
{
	class OSCReceiver;
	class OSCInputComponentInstance;

	/**
	 * Main interface for OSC messages in NAP
	 */
	class NAPAPI OSCService : public Service
	{
		friend class OSCReceiver;
		friend class OSCInputComponentInstance;
		RTTI_ENABLE(Service)
	public:
		// Default Constructor
		OSCService() = default;

		// Default Destructor
		virtual ~OSCService();

	protected:
		/**
		 * Registers all objects that need a specific way of construction
		 * @param factory the factory to register the object creators with
		 */
		virtual void registerObjectCreators(rtti::Factory& factory) override;

		// Initialization
		virtual bool init(nap::utility::ErrorState& errorState) override;

		/**
		* Processes all OSC received events
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
	};
}