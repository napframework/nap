#pragma once

#include "resource.h"
#include "event.h"
#include "utility/uniqueptrvectoriterator.h"

namespace nap
{
	/** 
	 * This is the Window abstraction that has no relationship to any physical or visible Window. 
	 * Instead it is meant to provide an abstract way of dealing with events, making it suitable
	 * for dealing with input and window related messages, without relying on any platform or graphics
	 * API.
	 */
	class WindowResource : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)

	public:
		using EventPtrList = std::vector<EventPtr>;
		using EventPtrConstIterator = utility::UniquePtrConstVectorWrapper<EventPtrList, Event*>;

		/**
		 * Adds an event to the queue, to be processed later.
		 * @param inEvent The event to add, ownership is transfered here.
		 */
		void addEvent(EventPtr inEvent);

		/**
		 * Processes the queue that was built up in addEvent. Sends all the queued events
		 * by signaling onEvent. After processing, the queue is cleared.
		 */
		void processEvents();

		/**
		 * @return all queued events. Queue is not cleared.
		 */
		EventPtrConstIterator GetEvents() const { return EventPtrConstIterator(mEvents); }

		Signal<const Event&> onEvent;	// Subscribe to this signal to respond to any events broadcasted by processEvents.

	private:		
		EventPtrList mEvents;			// Queue of all the events added by addEvent
	};
}
