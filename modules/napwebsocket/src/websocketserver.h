#pragma once

// Local Includes
#include "websocketserverendpoint.h"

// External Includes
#include <nap/resourceptr.h>
#include <queue>

namespace nap
{
	/**
	 * Interface for a web-socket server that listens to incoming web-socket events.
	 * Override the onEventReceived() method to provide a handler for received web socket events.
	 * Note that this object takes ownership of the event.
	 */
	class NAPAPI IWebSocketServer : public Resource
	{
		friend class WebSocketServerEndPoint;
		RTTI_ENABLE(Resource)
	public:
		// Stops the device
		virtual ~IWebSocketServer() override;

		/**
		* Initializes the interface.
		* @param errorState contains the error if the server can't be started
		* @return if the server started
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		nap::ResourcePtr<WebSocketServerEndPoint> mEndPoint = nullptr;		///< Property: 'EndPoint' link to the web-socket server end point

	protected:
		/**
		 * Called when a new web socket event is received from the web socket server end point
		 * Override this method to react on specific web socket events
		 * @param newEvent the event that is received
		 */
		virtual void onEventReceived(WebSocketEventPtr newEvent) = 0;

	private:
		/**
		 * Adds an event to the queue
		 * @param event the event to add, note that this receiver will take ownership of the event
		 */
		void eventReceived(WebSocketEventPtr newEvent);
	};


	//////////////////////////////////////////////////////////////////////////
	// WebSocketServer
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Allows for receiving and responding to messages over a web socket. Implements the IWebSocketServer interface.
	 * The server converts low-level web-socket messages into events that can be interpreted by the running application.
	 * Messages are received on a separate thread and consumed by the main thread.
	 */
	class NAPAPI WebSocketServer : public IWebSocketServer
	{
		RTTI_ENABLE(IWebSocketServer)
	public:

		/**
		 * Initializes the server.
		 * @param errorState contains the error if the server can't be started
		 * @return if the server started
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	protected:
		/**
		 * Called when the end point receives a new event.
		 * Adds the event to list of events to be processed later.
		 * @param newEvent the newly generated websocket event.
		 */
		virtual void onEventReceived(WebSocketEventPtr newEvent) override;

	private:
		// Queue that holds all the consumed events
		std::queue<WebSocketEventPtr> mEvents;

		// Mutex associated with setting / getting events
		std::mutex	mEventMutex;

		/**
		 * Consumes all received websocket events and moves them to outEvents
		 * Calling this will clear the internal queue and transfers ownership of the events to the caller
		 * @param outEvents will hold the transferred websocket events
		 */
		void consumeEvents(std::queue<WebSocketEventPtr>& outEvents);
	};
}
