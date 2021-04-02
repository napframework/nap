/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// local includes
#include "sequenceplayeroutput.h"
#include "sequenceevent.h"

// nap includes
#include <nap/resourceptr.h>
#include <nap/signalslot.h>

// external includes
#include <queue>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequenceService;

	/**
	 * SequencePlayerEventOutput dispatches an event on the timeline
	 */
	class NAPAPI SequencePlayerEventOutput : public SequencePlayerOutput
	{
		friend class SequencePlayerEventAdapter;

		RTTI_ENABLE(SequencePlayerOutput);
	public:
		/**
		 * Constructor
		 * @param service reference to SequenceService
		 */
		explicit SequencePlayerEventOutput(SequenceService& service);

	public:
		/**
		 * Signal will be triggered from main thread
		 */
		nap::Signal<const SequenceEventBase&> mSignal;
	protected:
		/**
		 * called from sequence service main thread
		 * @param deltaTime time since last update
		 */
		void update(double deltaTime) override ;

		/**
		 * called from sequence player thread, adds event to queue.
		 * @param event the event ptr that needs to be dispatched. Receiver takes ownership of the event
		 */
		void addEvent(SequenceEventPtr event);
	private:
		// the queue of events
		std::queue<SequenceEventPtr> mEvents;

		// thread mutex
		std::mutex mEventMutex;
	};

	using SequencePlayerEventOutputObjectCreator = rtti::ObjectCreator<SequencePlayerEventOutput, SequenceService>;
}