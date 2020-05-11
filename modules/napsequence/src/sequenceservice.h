#pragma once
// Local Includes
#include "sequenceplayer.h"

// External Includes
#include <nap/service.h>
#include <entity.h>
#include <nap/datetime.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequenceEventReceiver;
	class SequencePlayerParameterSetterBase;

	/**
	 * Main interface for processing sequence events and setting the parameters on the main thread
	 * All SequenceEventReceivers and ParameterSetters are registered and de-registered with this service on initialization and destruction
	 * This service consumes all received sequence events and forwards them to all registered SequenceEventReceivers
	 * Also, the service sets all parameter values that are tied to a sequenceplayer and its tracks when the user wants parameters to be set on main thread
	 */
	class NAPAPI SequenceService : public Service
	{
		friend class SequenceEventReceiver;
		friend class SequencePlayerParameterSetterBase;

		RTTI_ENABLE(Service)
	public:
		// Default Constructor
		SequenceService(ServiceConfiguration* configuration);

		// Default Destructor
		virtual ~SequenceService();

	protected:
		/**
		 * Registers all objects that need a specific way of construction
		 * @param factory the factory to register the object creators with
		 */
		virtual void registerObjectCreators(rtti::Factory& factory) override;

		/**
		 * init
		 * initializes service
		 * @param errorState contains any errors
		 * @return returns true on successful initialization
		 */
		virtual bool init(nap::utility::ErrorState& errorState) override;

		/**
		 * update
		 * updates any ParameterSetters and SequenceEventReceivers from main thread
		 * @param deltaTime deltaTime
		 */
		virtual void update(double deltaTime) override;
	private:
		/**
		 * registerEventReceiver
		 * registers a new event receiver
		 * @param receiver reference to event receiver
		 */
		void registerEventReceiver(SequenceEventReceiver& receiver);

		/**
		 * registerEventReceiver
		 * removes a registered event receiver
		 * @param receiver reference to event receiver
		 */
		void removeEventReceiver(SequenceEventReceiver& receiver);

		/**
		 * registerParameterSetter
		 * registers a new parameter setter 
		 * @param setter reference to parameter setter
		 */
		void registerParameterSetter(SequencePlayerParameterSetterBase& setter);

		/**
		 * removeParameterSetter
		 * removes a registered parameter setter 
		 * @param setter reference to parameter setter to be removed
		 */
		void removeParameterSetter(SequencePlayerParameterSetterBase& setter);

		// sequence event receivers
		std::vector<SequenceEventReceiver*>			mEventReceivers;

		// parameter setters
		std::vector<SequencePlayerParameterSetterBase*> mParameterSetters;
	};
}