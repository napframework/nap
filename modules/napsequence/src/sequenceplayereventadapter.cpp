#include "sequenceplayereventadapter.h"
#include "sequenceplayereventoutput.h"
#include "sequenceplayer.h"
#include "sequencetrackevent.h"

#include <nap/logger.h>

namespace nap
{
	static bool sRegisteredInFactory = SequencePlayerAdapter::registerFactory(RTTI_OF(SequenceTrackEvent), [](SequenceTrack& track, SequencePlayerOutput& output)->std::unique_ptr<SequencePlayerAdapter>
	{
		assert(output.get_type() == RTTI_OF(SequencePlayerEventOutput)); // type mismatch

		auto& eventOutput = static_cast<SequencePlayerEventOutput&>(output);

		auto adapter = std::make_unique<SequencePlayerEventAdapter>(track, eventOutput);
		return std::move(adapter);
	});


	SequencePlayerEventAdapter::SequencePlayerEventAdapter(SequenceTrack& track, SequencePlayerEventOutput& output)
		: mTrack(track), mOutput(output)
	{
	}


	void SequencePlayerEventAdapter::update(double time)
	{
		assert(mTrack.get_type().is_derived_from(RTTI_OF(SequenceTrackEvent)));
		auto& eventTrack = static_cast<SequenceTrackEvent&>(mTrack);
		for (const auto& eventSegment : eventTrack.mSegments)
		{
			assert(eventSegment.get()->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentEventBase)));
			SequenceTrackSegmentEventBase& event = static_cast<SequenceTrackSegmentEventBase&>(*eventSegment.get());

			if (time > event.mStartTime)
			{
				if (mDispatchedEvents.find(&event) == mDispatchedEvents.end())
				{
					mOutput.addEvent(event.createEvent());
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
