#pragma once

// Local Includes
#include "websocketserverendpoint.h"
#include "websocketinterface.h"

// External Includes
#include <queue>
#include <rtti/factory.h>
#include <nap/resourceptr.h>

namespace nap
{
	/**
	 * Interface for a web-socket server that listens to incoming web-socket events.
	 * Override the onEventReceived() method to provide a handler for newly received web socket events.
	 * The WebSocketServerEndPoint can have one or multiple links to an IWebSocketServer
	 */
	class NAPAPI IWebSocketServer : public WebSocketInterface
	{
		RTTI_ENABLE(WebSocketInterface)
	public:
		IWebSocketServer(WebSocketService& service);

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
		 * Sends a message with the given opcode to the specified client.
		 * @param connection the client connection
		 * @param message the message to send
		 * @param code message type
		 * @param error contains the error if sending fails
		 * @return if message was sent successfully
		 */
		bool send(const WebSocketConnection& connection, const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error);

		/**
		 * Sends a message using the given payload and opcode to the specified client.
		 * @param connection the client connection
		 * @param payload the message buffer
		 * @param length total number of bytes
		 * @param code message type
		 * @param error contains the error if sending fails
		 * @return if message was sent successfully
		 */
		bool send(const WebSocketConnection& connection, void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error);

		/**
		 * Sends a message to the specified client.
		 * @param connection the client connection
		 * @param message the message to send
		 * @param error contains the error if sending fails
		 * @return if message was sent successfully.
		 */
		bool send(const WebSocketConnection& connection, const WebSocketMessage& message, nap::utility::ErrorState& error);

	private:
		void onConnectionOpened(const WebSocketConnection& connection);
		nap::Slot<const WebSocketConnection&> mConnectionOpened;

		void onConnectionClosed(const WebSocketConnection& connection, int code, const std::string& reason);
		nap::Slot<const WebSocketConnection&, int, const std::string&> mConnectionClosed;

		void onConnectionFailed(const WebSocketConnection& connection, int code, const std::string& reason);
		nap::Slot<const WebSocketConnection&, int, const std::string&> mConnectionFailed;

		void onMessageReceived(const WebSocketConnection& connection, const WebSocketMessage& message);
		nap::Slot<const WebSocketConnection&, const WebSocketMessage&> mMessageReceived;
	};

	// Object creator used for constructing the websocket server
	using WebSocketServerObjectCreator = rtti::ObjectCreator<WebSocketServer, WebSocketService>;
}
