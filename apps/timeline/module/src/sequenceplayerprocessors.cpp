#include "sequenceplayerprocessors.h"
#include "sequenceservice.h"
#include "sequenceeventreceiver.h"

namespace nap
{
	SequencePlayerParameterSetterBase::SequencePlayerParameterSetterBase(SequenceService& service)
		: mService(service)
	{
		mService.registerParameterSetter(*this);
	}


	SequencePlayerParameterSetterBase::~SequencePlayerParameterSetterBase()
	{
		mService.removeParameterSetter(*this);
	}


	SequencePlayerProcessorEvent::SequencePlayerProcessorEvent(SequenceTrack& track, SequenceEventReceiver& receiver)
		: mTrack(track), mReceiver(receiver)
	{

	}


	void SequencePlayerProcessorEvent::process(double time)
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
				break;;
			}
		}
	}
}
