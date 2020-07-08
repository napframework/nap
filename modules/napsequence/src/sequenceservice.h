#pragma once

// Local Includes
#include "sequenceplayer.h"

// External Includes
#include <nap/service.h>
#include <entity.h>
#include <nap/datetime.h>
#include <rtti/factory.h>

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
		friend class SequencePlayerOutput;

		RTTI_ENABLE(Service)
	public:
		// Default Constructor
		SequenceService(ServiceConfiguration* configuration);

		// Default Destructor
		virtual ~SequenceService();

		static bool registerObjectCreator(std::unique_ptr<rtti::IObjectCreator>(*objectCreator)(SequenceService*));
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
		 * registers an output
		 * @param input reference to output
		 */
		void registerOutput(SequencePlayerOutput& output);

		/**
		 * removes an input
		 * @param input reference to input
		 */
		void removeOutput(SequencePlayerOutput& output);

		//
		std::vector<SequencePlayerOutput*> mOutputs;
	};
}