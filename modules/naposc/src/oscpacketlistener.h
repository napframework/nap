#pragma once

// Local Includes
#include "oscevent.h"

// External Includes
#include <osc/OscPacketListener.h>
#include <queue>
#include <mutex>

namespace nap
{
	class OSCReceiver;

	/**
	 *	Listens to incoming packages and translates those in to OSC events
	 */
	class NAPAPI OSCPacketListener : public osc::OscPacketListener
	{
		friend class OSCReceiver;
	public:
		/**
		* Moves all the received OSC events over to @outEvents
		* @param outEvents the events to populate
		*/
		void consumeEvents(std::queue<OSCEventPtr>& outEvents);

	protected:
		virtual void ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint) override;

	private:
		/**
		 * Adds a new OSC event to the queue, thread safe
		 * @param event the osc event to add
		 */
		void addEvent(OSCEventPtr event);

		// Holds all the received OSC messages as parsed events
		std::queue<OSCEventPtr> mReceivedEvents;

		// Mutex associated with setting / getting events
		std::mutex	mEventMutex;
	};
}
