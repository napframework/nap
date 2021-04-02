/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequenceplayereventadapter.h"
#include "sequenceplayereventoutput.h"
#include "sequenceplayer.h"
#include "sequencetrackevent.h"

#include <nap/logger.h>

namespace nap
{
	static bool sRegisteredInFactory = SequencePlayerAdapter::registerFactory(RTTI_OF(SequenceTrackEvent), [](const SequenceTrack& track, SequencePlayerOutput& output, const SequencePlayer& player)->std::unique_ptr<SequencePlayerAdapter>
	{
		assert(output.get_type() == RTTI_OF(SequencePlayerEventOutput)); // type mismatch

		auto& eventOutput = *rtti_cast<SequencePlayerEventOutput>(&output);

		auto adapter = std::make_unique<SequencePlayerEventAdapter>(track, eventOutput, player);
		return std::move(adapter);
	});


	SequencePlayerEventAdapter::SequencePlayerEventAdapter(const SequenceTrack& track, SequencePlayerEventOutput& output, const SequencePlayer& player)
		: mTrack(track), mOutput(output)
	{
		double time = player.getPlayerTime();

		// mark all events before 'time' as already dispatched
		assert(mTrack.get_type().is_derived_from(RTTI_OF(SequenceTrackEvent)));
		const auto* event_track = rtti_cast<const SequenceTrackEvent>(&mTrack);
		for (const auto& event_segment : event_track->mSegments)
		{
			assert(event_segment.get()->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentEventBase)));
			auto& event = *rtti_cast<SequenceTrackSegmentEventBase>(event_segment.get());
			if (time > event.mStartTime)
			{
				if (mDispatchedEvents.find(&event) == mDispatchedEvents.end())
				{
					mDispatchedEvents.emplace(&event);
				}
			}
		}
	}


	void SequencePlayerEventAdapter::tick(double time)
	{
		double deltaTime = time - mPrevTime;
		mPrevTime = time;
		if (deltaTime < 0.0)
		{
			if (!mPlayingBackwards)
			{
				mPlayingBackwards = true;

				//
				mDispatchedEvents.clear();

				// mark all events after 'time' as already dispatched
				assert(mTrack.get_type().is_derived_from(RTTI_OF(SequenceTrackEvent)));
				const auto& event_track = *rtti_cast<const SequenceTrackEvent>(&mTrack);
				for (const auto& event_segment : event_track.mSegments)
				{
					assert(event_segment.get()->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentEventBase)));
					auto& event = *rtti_cast<SequenceTrackSegmentEventBase>(event_segment.get());
					if (time < event.mStartTime)
					{
						if (mDispatchedEvents.find(&event) == mDispatchedEvents.end())
						{
							mDispatchedEvents.emplace(&event);
						}
					}
				}
			}
		}
		else
		{
			if (mPlayingBackwards)
			{
				mPlayingBackwards = false;

				//
				mDispatchedEvents.clear();

				// mark all events before 'time' as already dispatched
				assert(mTrack.get_type().is_derived_from(RTTI_OF(SequenceTrackEvent)));
				const auto& event_track = *rtti_cast<const SequenceTrackEvent>(&mTrack);
				for (const auto& event_segment : event_track.mSegments)
				{
					assert(event_segment.get()->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentEventBase)));
					auto& event = *rtti_cast<SequenceTrackSegmentEventBase>(event_segment.get());
					if (time > event.mStartTime)
					{
						if (mDispatchedEvents.find(&event) == mDispatchedEvents.end())
						{
							mDispatchedEvents.emplace(&event);
						}
					}
				}
			}
		}

		assert(mTrack.get_type().is_derived_from(RTTI_OF(SequenceTrackEvent)));
		const auto& event_track = *rtti_cast<const SequenceTrackEvent>(&mTrack);
		for (const auto& event_segment : event_track.mSegments)
		{
			assert(event_segment.get()->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentEventBase)));
			auto& event = *rtti_cast<SequenceTrackSegmentEventBase>(event_segment.get());

			if ( ( !mPlayingBackwards && time > event.mStartTime ) || (mPlayingBackwards && time < event.mStartTime))
			{
				if (mDispatchedEvents.find(&event) == mDispatchedEvents.end())
				{
					mOutput.addEvent(event.createEvent());
					mDispatchedEvents.emplace(&event);
				}
			}
			else if ((!mPlayingBackwards && time < event.mStartTime) || (mPlayingBackwards && time > event.mStartTime))
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
