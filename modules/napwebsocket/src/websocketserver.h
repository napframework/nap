#pragma once

// Local Includes
#include "websocketserverendpoint.h"
#include "websocketevents.h"

// External Includes
#include <queue>
#include <rtti/factory.h>
#include <nap/resourceptr.h>

namespace nap
{
	class WebSocketService;

	/**
	 * Interface for a web-socket server that listens to incoming web-socket events.
	 * Override the onEventReceived() method to provide a handler for newly received web socket events.
	 * The WebSocketServerEndPoint can have one or multiple links to an IWebSocketServer
	 */
	class NAPAPI IWebSocketServer : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		 * Registers the server with the end point.
		 * @param errorState contains the error if initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		ResourcePtr<WebSocketServerEndPoint> mEndPoint;	///< Property: 'EndPoint' the server end point that manages the connections
	};


	//////////////////////////////////////////////////////////////////////////
	// WebSocketServer
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Allows for receiving and responding to messages over a web socket. Implements the IWebSocketServer interface.
	 * The server receives web socket events that can be interpreted by the running application.
	 * Events are received on a separate thread and consumed on the main thread by the WebSocketService.
	 */
	class NAPAPI WebSocketServer : public IWebSocketServer
	{
		friend class WebSocketService;
		RTTI_ENABLE(IWebSocketServer)
	public:

		// Constructor used by factory
		WebSocketServer(WebSocketService& service);

		// Destructor
		virtual ~WebSocketServer();

		/**
		 * Initializes the server.
		 * @param errorState contains the error if the server can't be started
		 * @return if the server started
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Sends a message to the incoming connection
		 * @param connection the client connection
		 * @param message the message to send
		 * @param code message type
		 * @param error contains the error if sending fails
		 * @return if message was send successfully
		 */
		bool send(WebSocketConnection connection, const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error);

		/**
		 * Sends a message using the given payload and opcode
		 * @param connection the client connection
		 * @param payload the message buffer
		 * @param length total number of bytes
		 * @param code message type
		 * @param error contains the error if sending fails
		 * @return if message was send successfully
		 */
		bool send(WebSocketConnection connection, void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error);

	private:
		// Queue that holds all the consumed events
		std::queue<WebSocketEventPtr> mEvents;

		// Mutex associated with setting / getting events
		std::mutex	mEventMutex;

		/**
		 * Consumes all received web-socket events and moves them to outEvents
		 * Calling this will clear the internal queue and transfers ownership of the events to the caller
		 * @param outEvents will hold the transferred web-socket events
		 */
		void consumeEvents(std::queue<WebSocketEventPtr>& outEvents);

		// Handle to the web socket service
		WebSocketService* mService = nullptr;

		void onConnectionOpened(WebSocketConnection connection);
		nap::Slot<WebSocketConnection> mConnectionOpened;

		void onConnectionClosed(WebSocketConnection connection, int code, const std::string& reason);
		nap::Slot<WebSocketConnection, int, const std::string&> mConnectionClosed;

		void onConnectionFailed(WebSocketConnection connection, int code, const std::string& reason);
		nap::Slot<WebSocketConnection, int, const std::string&> mConnectionFailed;

		void onMessageReceived(WebSocketConnection connection, WebSocketMessage message);
		nap::Slot<WebSocketConnection, WebSocketMessage> mMessageReceived;

		/**
		 * Called when the end point receives a new event.
		 * Adds the event to the list of events to be processed on the main thread.
		 * @param newEvent the web-socket event.
		 */
		void addEvent(WebSocketEventPtr newEvent);
	};

	// Object creator used for constructing the the websocket server
	using WebSocketServerObjectCreator = rtti::ObjectCreator<WebSocketServer, WebSocketService>;
}
