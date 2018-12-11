
#include "timeline.h"

bool nap::TimelineComponentInstance::init(nap::utility::ErrorState& errorState)
{
	// TODO: get sorted (by start time) list of events and insert into mQueuedEvents

	for (auto& eventResource : mTimelineComponent->events())
		mQueuedEvents.emplace_back(eventResource->create());
	std::sort(mQueuedEvents.begin(), mQueuedEvents.end(),
			  [](const std::unique_ptr<TimelineEventInstance>& a, const std::unique_ptr<TimelineEventInstance>& b)
			  {
				  return a->getResource()->mStartTime < b->getResource()->mStartTime;
			  });

	return ComponentInstance::init(errorState);
}

void nap::TimelineComponentInstance::update(double deltaTime)
{
	double localDelta = deltaTime * mSpeed;
	mTime += localDelta;

	startNewEvents();
	updateActiveEvents(localDelta);
	endOldEvents();
}

void nap::TimelineComponentInstance::startNewEvents()
{
	for (size_t i = mQueuedEvents.size() - 1; i > 0; i--)
	{
		auto& queuedEvent = mQueuedEvents[i];
		// is this event ready to be started?
		if (mTime > queuedEvent->getResource()->mStartTime)
		{
			// initialize event
			queuedEvent->start();

			// move event from queued to active
			mActiveEvents.emplace_back(std::move(queuedEvent));
			mQueuedEvents.erase(mQueuedEvents.begin() + i);
		}
	}
}

void nap::TimelineComponentInstance::updateActiveEvents(double deltaTime)
{
	for (auto& activeEvent : mActiveEvents)
		activeEvent->update(deltaTime);
}

void nap::TimelineComponentInstance::endOldEvents()
{
	for (size_t i = mActiveEvents.size() - 1; i > 0; i--)
	{
		auto& queuedEvent = mQueuedEvents[i];
		// is this event ready to be ended?
		if (mTime > queuedEvent->getResource()->mEndTime)
		{
			// finalize event
			queuedEvent->end();

			// move event from active to finished (?)
//			mFinishedEvents.emplace_back(std::move(queuedEvent));

			// FIXME: this is a bug! Probably store active events in a sorted set
			mActiveEvents.erase(mQueuedEvents.begin() + i);
		}
	}
}
