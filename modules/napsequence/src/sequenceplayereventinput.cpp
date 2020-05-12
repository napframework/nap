#include "sequenceplayereventinput.h"
#include "sequenceeventreceiver.h"
#include "sequenceservice.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequencePlayerEventInput)
RTTI_PROPERTY("Event Receiver", &nap::SequencePlayerEventInput::mReceiver, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	static bool registerObjectCreator = SequenceService::registerObjectCreator([](SequenceService* service)->std::unique_ptr<rtti::IObjectCreator>
	{
		return std::make_unique<SequencePlayerEventInputObjectCreator>(*service);
	});

	SequencePlayerEventInput::SequencePlayerEventInput(SequenceService& service)
		: SequencePlayerInput(service){}

	void SequencePlayerEventInput::update(double deltaTime)
	{
		std::queue<SequenceEventPtr> events;

		// Forward every event to every input component of interest
		mReceiver->consumeEvents(events);

		// Keep forwarding events until the queue runs out
		while (!(events.empty()))
		{
			SequenceEventBase& sequence_event = *(events.front());
			mReceiver->mSignal.trigger(sequence_event);

			events.pop();
		}
	}
}