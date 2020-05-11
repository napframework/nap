// External Includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <iostream>
#include <utility/stringutils.h>

// Local Includes
#include "sequenceservice.h"
#include "sequenceeventreceiver.h"
#include "sequenceplayeradapter.h"
#include "sequenceplayercurveadapter.h"

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
		factory.addObjectCreator(std::make_unique<SequencePlayerObjectCreator>(*this));
		factory.addObjectCreator(std::make_unique<SequencePlayerCurveInputObjectCreator>(*this));
	}

	bool SequenceService::init(nap::utility::ErrorState& errorState)
	{
		return true;
	}

	void SequenceService::update(double deltaTime)
	{
		std::queue<SequenceEventPtr> events;

		// Forward every event to every input component of interest
		for (auto& receiver : mEventReceivers)
		{
			receiver->consumeEvents(events);

			// Keep forwarding events until the queue runs out
			while (!(events.empty()))
			{
				SequenceEventBase& sequence_event = *(events.front());
				receiver->mSignal.trigger(sequence_event);

				events.pop();
			}
		}

		//
		for (auto& setter : mParameterSetters)
		{
			setter->setValue();
		}
	}

	void SequenceService::registerEventReceiver(SequenceEventReceiver& receiver)
	{
		mEventReceivers.emplace_back(&receiver);
	}


	void SequenceService::removeEventReceiver(SequenceEventReceiver& receiver)
	{
		auto found_it = std::find_if(mEventReceivers.begin(), mEventReceivers.end(), [&](const auto& it)
		{
			return it == &receiver;
		});
		assert(found_it != mEventReceivers.end());
		mEventReceivers.erase(found_it);
	}

	void SequenceService::registerParameterSetter(SequencePlayerParameterSetterBase& setter)
	{
		mParameterSetters.emplace_back(&setter);
	}

	void SequenceService::removeParameterSetter(SequencePlayerParameterSetterBase& setter)
	{
		auto found_it = std::find_if(mParameterSetters.begin(), mParameterSetters.end(), [&](const auto& it)
		{
			return it == &setter;
		});
		assert(found_it != mParameterSetters.end());
		mParameterSetters.erase(found_it);
	}
}