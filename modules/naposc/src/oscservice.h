#pragma once
// Local Includes
#include "oscevent.h"

// External Includes
#include <nap/service.h>
#include <nap/entity.h>
#include <nap/timer.h>

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

		// Initialization
		bool init(nap::utility::ErrorState& errorState);

		/**
		 * Processes all OSC received events
		 */
		void update();

	protected:
		/**
		 * Registers all objects that need a specific way of construction
		 * @param factory the factory to register the object creators with
		 */
		virtual void registerObjectCreators(rtti::Factory& factory) override;

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

		/**
		 * Recursively collects all input components currently available in the system
		 * @param entity the entity to start iterating from
		 */
		void collectInputComponents(const EntityInstance& entity, std::vector<OSCInputComponentInstance*>& components);

		// All the osc receivers currently registered in the system
		std::vector<OSCReceiver*> mReceivers;

		// All the osc components currently available to the system
		std::vector<OSCInputComponentInstance*> mInputs;
	};
}