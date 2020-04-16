#pragma once
// Local Includes
#include "sequenceplayer.h"

// External Includes
#include <nap/service.h>
#include <entity.h>
#include <nap/datetime.h>

namespace nap
{
	class SequenceEventReceiver;

	class NAPAPI SequenceService : public Service
	{
		friend class SequenceEventReceiver;

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

		virtual bool init(nap::utility::ErrorState& errorState) override;

		virtual void update(double deltaTime) override;
	private:
		void registerReceiver(SequenceEventReceiver& receiver);

		void removeReceiver(SequenceEventReceiver& receiver);

		std::vector<SequenceEventReceiver*> mReceivers;
	};
}