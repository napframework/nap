#pragma once

// Local Includes
#include "websocketevent.h"

// External Includes
#include <nap/resource.h>
#include <queue>
#include <mutex>

namespace nap
{
	class WebSocketService;

	/**
	 * websocketinterface
	 */
	class NAPAPI WebSocketInterface : public Resource
	{
		friend class WebSocketService;
		RTTI_ENABLE(Resource)
	public:
		WebSocketInterface(WebSocketService& service);

		virtual ~WebSocketInterface();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

	protected:
		/**
		 * Called when the end point receives a new event.
		 * Adds the event to the list of events to be processed on the main thread.
		 * @param newEvent the web-socket event.
		 */
		void addEvent(WebSocketEventPtr newEvent);

		// Queue that holds all the consumed events
		std::queue<WebSocketEventPtr> mEvents;

		// Mutex associated with setting / getting events
		std::mutex	mEventMutex;

		// Handle to the web socket service
		WebSocketService* mService = nullptr;

	private:
		/**
		 * Consumes all received web-socket events and moves them to outEvents
		 * Calling this will clear the internal queue and transfers ownership of the events to the caller
		 * @param outEvents will hold the transferred web-socket events
		 */
		void consumeEvents(std::queue<WebSocketEventPtr>& outEvents);
	};
}
