#pragma once

// External Includes
#include <nap/service.h>
#include <nap/entity.h>

namespace nap
{
	class OSCReceiver;

	/**
	 * Main interface for OSC messages in NAP
	 */
	class NAPAPI OSCService : public Service
	{
		friend class OSCReceiver;
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
		 * @param entities: the entities to forward the messages to
		 */
		void processEvents(const EntityList& entities);

		/**
		 *	Processes all OSC received events for all active entities, starting with the root
		 */
		void processEvents();

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
		 *	Forwards all events to osc input components
		 */
		void forwardEvents(OSCReceiver& receiver, const EntityList& entities);

		// All the osc receivers currently registered in the system
		std::vector<OSCReceiver*> mReceivers;

		// Root entity
		nap::EntityInstance* mRootEntity = nullptr;
	};
}