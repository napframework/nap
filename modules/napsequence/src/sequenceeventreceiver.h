#pragma once

// internal includes
#include "sequenceevent.h"

// external includes
#include <nap/resource.h>
#include <nap/signalslot.h>
#include <rtti/factory.h>
#include <queue>
#include <mutex>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// forward declars
	class SequenceService;

	/**
	 * SequenceEventReceiver
	 * Receives events from a SequencePlayerEventAdapter and SequenceService dispatches them from the main thread
	 */
	class NAPAPI SequenceEventReceiver : public Resource
	{
		friend class SequencePlayerEventAdapter;
		friend class SequenceService;

		RTTI_ENABLE(Resource)
	public:
		/**
		 * Constructor
		 * Upon construction, this receiver registers itself to the service
		 * @param service reference to SequenceService
		 */
		SequenceEventReceiver(SequenceService& service);

		/**
		 * Deconstructor
		 * Upon deconstruction receiver removes itself from service
		 */
		virtual ~SequenceEventReceiver();

		/**
		 * Signal will be triggered from main thread
		 */
		nap::Signal<const SequenceEvent&> mSignal;
	private:
		/**
		 * consumeEvents
		 * called from SequenceService main thread
		 * @param outEvents will contain all events that need to be dispatched
		 */
		void consumeEvents(std::queue<SequenceEventPtr>& outEvents);

		/**
		 * addEvent
		 * called from sequence player thread, adds event to queue
		 * @param the event ptr that needs to be dispatched. Receiver takes ownership of the event
		 */
		void addEvent(SequenceEventPtr event);

		// the queue of events
		std::queue<SequenceEventPtr> mEvents;

		// thread mutex
		std::mutex mEventMutex;

		// pointer to service
		SequenceService* mService;
	};

	using SequenceReceiverObjectCreator = rtti::ObjectCreator<SequenceEventReceiver, SequenceService>;
}
