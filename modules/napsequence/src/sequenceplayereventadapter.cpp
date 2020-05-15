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


	void SequencePlayerEventAdapter::tick(double time)
	{
		assert(mTrack.get_type().is_derived_from(RTTI_OF(SequenceTrackEvent)));
		auto& event_track = static_cast<SequenceTrackEvent&>(mTrack);
		for (const auto& event_segment : event_track.mSegments)
		{
			assert(event_segment.get()->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentEventBase)));
			SequenceTrackSegmentEventBase& event = static_cast<SequenceTrackSegmentEventBase&>(*event_segment.get());

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
		for (auto* dispatched_event : mDispatchedEvents)
		{
			bool found_segment = false;
			for (const auto& event_segment : event_track.mSegments)
			{
				if (event_segment.get() == dispatched_event)
				{
					found_segment = true;
					break;
				}
			}

			if (!found_segment)
			{
				mDispatchedEvents.erase(dispatched_event);
				break;
			}
		}
	}
}
