// External Includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <iostream>
#include <utility/stringutils.h>

// Local Includes
#include "sequenceservice.h"
#include "sequenceeventreceiver.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceService)
RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	SequenceService::SequenceService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}

	SequenceService::~SequenceService()
	{ }

	void SequenceService::registerObjectCreators(rtti::Factory& factory)
	{
		factory.addObjectCreator(std::make_unique<SequenceReceiverObjectCreator>(*this));
	}

	bool SequenceService::init(nap::utility::ErrorState& errorState)
	{
		return true;
	}

	void SequenceService::update(double deltaTime)
	{
		std::queue<SequenceEventPtr> events;

		// Forward every event to every input component of interest
		for (auto& receiver : mReceivers)
		{
			receiver->consumeEvents(events);

			// Keep forwarding events until the queue runs out
			while (!(events.empty()))
			{
				SequenceEvent& sequence_event = *(events.front());
				receiver->mSignal.trigger(sequence_event);

				events.pop();
			}
		}
	}

	void SequenceService::registerReceiver(SequenceEventReceiver& receiver)
	{
		mReceivers.emplace_back(&receiver);
	}


	void SequenceService::removeReceiver(SequenceEventReceiver& receiver)
	{
		auto found_it = std::find_if(mReceivers.begin(), mReceivers.end(), [&](const auto& it)
		{
			return it == &receiver;
		});
		assert(found_it != mReceivers.end());
		mReceivers.erase(found_it);
	}
}