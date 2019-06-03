#pragma once

// Local Includes
#include "websocketserverendpoint.h"
#include "websocketevents.h"

// External Includes
#include <nap/resourceptr.h>
#include <queue>

namespace nap
{
	class WebSocketServerEndPoint;

	/**
	 * Allows for receiving and responding to messages over a web socket.
	 * The server converts low-level web-socket messages into events that can be interpreted by the running application.
	 * Messages are received on a separate thread and consumed by the main thread.
	 */
	class NAPAPI WebSocketServer : public Resource
	{
		RTTI_ENABLE(Resource)
	public:

		// Stops the device
		virtual ~WebSocketServer() override;

		/**
		 * Initializes the server
		 * @param errorState contains the error if the server can't be started
		 * @return if the server started
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		nap::ResourcePtr<WebSocketServerEndPoint> mEndPoint = nullptr;		///< Property: 'EndPoint' link to the web-socket server end point

	private:
		/**
		 * Called when the end point receives a message
		 * Converts the message into a WebSocketMessageEvent
		 */
		void onMessageReceived(WebSocketMessage message);
		nap::Slot<WebSocketMessage>	mMessageReceivedSlot						= { this, &WebSocketServer::onMessageReceived };

		void onConnectionClosed(WebSocketConnection connection);
		nap::Slot<WebSocketConnection> mConnectionClosedSlot					= { this, &WebSocketServer::onConnectionClosed};

		void onConnectionOpened(WebSocketConnection connection);
		nap::Slot<WebSocketConnection> mConnectionOpenedSlot					= { this, &WebSocketServer::onConnectionOpened };

		void onConnectionFailed(WebSocketConnection connection);
		nap::Slot<WebSocketConnection> mConnectionFailedSlot					= { this, &WebSocketServer::onConnectionFailed };

		// Queue that holds all the consumed events
		std::queue<WebSocketEventPtr> mEvents;

		// Mutex associated with setting / getting events
		std::mutex	mEventMutex;

		/**
		 * Consumes all received OSC events and moves them to outEvents
		 * Calling this will clear the internal queue and transfers ownership of the events to the caller
		 * @param outEvents will hold the transferred osc events
		 */
		void consumeEvents(std::queue<WebSocketEventPtr>& outEvents);

		/**
		 * Adds an event to the queue
		 * @param event the event to add, note that this receiver will take ownership of the event
		 */
		void addEvent(WebSocketEventPtr newEvent);
	};
}
