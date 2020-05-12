#include "sequenceeventreceiver.h"
#include "sequenceservice.h"

RTTI_BEGIN_CLASS(nap::SequenceEventReceiver)
RTTI_END_CLASS

namespace nap
{
	void SequenceEventReceiver::consumeEvents(std::queue<SequenceEventPtr>& outEvents)
	{
		std::lock_guard<std::mutex> lock(mEventMutex);

		// Swap events
		outEvents.swap(mEvents);

		// Clear current queue
		std::queue<SequenceEventPtr> empty_queue;
		mEvents.swap(empty_queue);
	}


	void SequenceEventReceiver::addEvent(SequenceEventPtr event)
	{
		std::lock_guard<std::mutex> lock(mEventMutex);
		mEvents.emplace(std::move(event));
	}
}