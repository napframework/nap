#include "sequenceplayereventadapter.h"
#include "sequenceeventreceiver.h"
#include "sequenceplayer.h"

#include <nap/logger.h>

namespace nap
{
	static bool sRegisteredInFactory = SequencePlayerAdapter::registerFactory(RTTI_OF(SequenceTrackEvent), [](SequenceTrack& track, SequencePlayerInput& input)->std::unique_ptr<SequencePlayerAdapter>
	{
		assert(input.get_type() == RTTI_OF(SequencePlayerEventInput)); // type mismatch

		auto& eventInput = static_cast<SequencePlayerEventInput&>(input);

		auto adapter = std::make_unique<SequencePlayerEventAdapter>(track, *eventInput.mReceiver.get());
		return std::move(adapter);
	});


	SequencePlayerEventAdapter::SequencePlayerEventAdapter(SequenceTrack& track, SequenceEventReceiver& receiver)
		: mTrack(track), mReceiver(receiver)
	{
	}


	void SequencePlayerEventAdapter::update(double time)
	{
		assert(mTrack.get_type().is_derived_from(RTTI_OF(SequenceTrackEvent)));
		auto& eventTrack = static_cast<SequenceTrackEvent&>(mTrack);
		for (const auto& eventSegment : eventTrack.mSegments)
		{
			assert(eventSegment.get()->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentEvent)));
			SequenceTrackSegmentEvent& event = static_cast<SequenceTrackSegmentEvent&>(*eventSegment.get());

			if (time > event.mStartTime)
			{
				if (mDispatchedEvents.find(&event) == mDispatchedEvents.end())
				{
					mReceiver.addEvent(event.createEvent());
					mDispatchedEvents.emplace(&event);
				}
			}
			else if (time < event.mStartTime)
			{
				if (mDispatchedEvents.find(&event) != mDispatchedEvents.end())
				{
					mDispatchedEvents.erase(&event);
				}
			}
		}

		// remove dispatchedEvents that have been deleted
		for (auto* dispatchedEvent : mDispatchedEvents)
		{
			bool foundSegment = false;
			for (const auto& eventSegment : eventTrack.mSegments)
			{
				if (eventSegment.get() == dispatchedEvent)
				{
					foundSegment = true;
					break;
				}
			}

			if (!foundSegment)
			{
				mDispatchedEvents.erase(dispatchedEvent);
				break;
			}
		}
	}
}
