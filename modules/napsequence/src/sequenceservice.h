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
	 * Main interface for processing sequence events and updating sequenceoutputs
	 * F.E. 
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

		/**
		 * Registers object create for specific output types
		 * Each output type must register its own factory method so its constructor gets initialized with a reference to the sequenceservice
		 */
		static bool registerObjectCreator(std::unique_ptr<rtti::IObjectCreator>(*objectCreator)(SequenceService*));
	protected:
		/**
		 * Registers all objects that need a specific way of construction
		 * @param factory the factory to register the object creators with
		 */
		virtual void registerObjectCreators(rtti::Factory& factory) override;

		/**
		 * initializes service
		 * @param errorState contains any errors
		 * @return returns true on successful initialization
		 */
		virtual bool init(nap::utility::ErrorState& errorState) override;

		/**
		 * updates any outputs from main thread
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