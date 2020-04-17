#pragma once

// internal includes
#include "sequenceevent.h"
#include "sequenceservice.h"

// external includes
#include <nap/resource.h>
#include <nap/signalslot.h>
#include <rtti/factory.h>
#include <queue>
#include <mutex>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * 
	 */
	class NAPAPI SequenceEventReceiver : public Resource
	{
		friend class SequencePlayerProcessorEvent;
		friend class SequenceService;

		RTTI_ENABLE(Resource)
	public:
		// Constructor used by factory
		SequenceEventReceiver(SequenceService& service);

		virtual ~SequenceEventReceiver();

		nap::Signal<const SequenceEvent&> mSignal;
	private:
		/**
		 * called from SequenceService main thread
		 */
		void consumeEvents(std::queue<SequenceEventPtr>& outEvents);

		/**
		 * called from sequence player thread
		 */
		void addEvent(SequenceEventPtr event);

		std::queue<SequenceEventPtr> mEvents;

		std::mutex mEventMutex;

		SequenceService* mService;
	};

	using SequenceReceiverObjectCreator = rtti::ObjectCreator<SequenceEventReceiver, SequenceService>;
}
