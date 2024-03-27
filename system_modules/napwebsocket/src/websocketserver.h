/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "iwebsocketserverendpoint.h"
#include "websocketinterface.h"

// External Includes
#include <rtti/factory.h>
#include <nap/resourceptr.h>

namespace nap
{
	/**
	 * Interface for all web-socket servers.
	 * Derive from this class to implement your own web-socket server.
	 * On initialization the client registers itself with a nap::WebSocketClientEndPointBase.
	 */
	class NAPAPI IWebSocketServer : public WebSocketInterface
	{
		RTTI_ENABLE(WebSocketInterface)
	public:
		/**
		 * Constructor
		 * @param service the web-socket service that forwards events to the application.
		 */
		IWebSocketServer(WebSocketService& service);

		/**
		 * Registers the web-socket server interface with the endpoint.
		 * @param errorState contains the error if initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Unregisters the web-socket server interface with the endpoint.
		 */
		virtual void onDestroy() override;

		// Called by web-socket server endpoint when a new message is received
		virtual void onMessageReceived(const WebSocketConnection& connection, const WebSocketMessage& message) = 0;

		// Called by web-socket server endpoint when a client connection opened
		virtual void onConnectionOpened(const WebSocketConnection& connection) = 0;

		// Called by web-socket server endpoint when a client connection closed
		virtual void onConnectionClosed(const WebSocketConnection& connection, int code, const std::string& reason) = 0;

		// Called by web-socket server endpoint when a client connection failed to establish
		virtual void onConnectionFailed(const WebSocketConnection& connection, int code, const std::string& reason) = 0;

		ResourcePtr<IWebSocketServerEndPoint> mEndPoint;	///< Property: 'EndPoint' the server endpoint that manages all client connections
	};


	//////////////////////////////////////////////////////////////////////////
	// WebSocketServer
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Default implementation of a web-socket server.
	 * Receives and responds to client messages over a web socket and can
	 * be used to send a reply. Implements the IWebSocketServer interface.
	 * The server converts raw messages and connection updates from a nap::WebSocketServerEndPoint 
	 * into web-socket events that can be interpreted by the running application. 
	 * Events are generated on a background thread and consumed on the main thread on update(). 
	 * Use a nap::WebSocketComponent to receive and react to client web-socket events in your application.
	 */
	class NAPAPI WebSocketServer : public IWebSocketServer
	{
		friend class WebSocketService;
		RTTI_ENABLE(IWebSocketServer)
	public:
		/**
		 * Constructor
		 * @param service the web-socket service that forwards events to the application.
		 */
		WebSocketServer(WebSocketService& service);

		/**
		 * Sends a message to a client.
		 * @param connection the client connection
		 * @param message the message to send
		 * @param error contains the error if sending fails
		 * @return if message was sent successfully.
		 */
		bool send(const WebSocketConnection& connection, const WebSocketMessage& message, nap::utility::ErrorState& error);

		/**
		 * Sends a message using the given payload and opcode to a client.
		 * @param connection the client connection
		 * @param payload the message buffer
		 * @param length total number of bytes
		 * @param code message type
		 * @param error contains the error if sending fails
		 * @return if message was sent successfully
		 */
		bool send(const WebSocketConnection& connection, void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error);

		/**
		 * Sends a message with the given opcode to a client.
		 * @param connection the client connection
		 * @param message the message to send
		 * @param code message type
		 * @param error contains the error if sending fails
		 * @return if message was sent successfully
		 */
		bool send(const WebSocketConnection& connection, const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error);

		/**
		 * Broadcasts a message to all connected clients
		 * @param message the message to send
		 * @param error contains the error if sending fails
		 * @return if message was broadcast successfully
		 */
		bool broadcast(const WebSocketMessage& message, nap::utility::ErrorState& error);

		/**
		 * Broadcasts a message to all connected clients
		 * @param message the message to send
		 * @param error contains the error if sending fails
		 * @return if message was broadcast successfully
		 */
		bool broadcast(void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error);

	private:
		/**
		 * Called by web-socket server endpoint when the connection is opened.
		 * Generates and forwards a nap::WebSocketConnectionEvent to the running application on the main thread.
		 * @param connection client connection
		 */
		virtual void onConnectionOpened(const WebSocketConnection& connection) override;

		/**
		 * Called by web-socket server endpoint when the connection is closed.
		 * Generates and forwards a nap::WebSocketConnectionClosedEvent to the running application on the main thread.
		 * @param connection client connection
		 * @param code the closing code
		 * @param reason the reason why the connection was closed.
		 */
		virtual void onConnectionClosed(const WebSocketConnection& connection, int code, const std::string& reason) override;

		/**
		 * Called by web-socket server endpoint when the connection failed to establish.
		 * Generates and forwards a nap::WebSocketConnectionFailedEvent to the running application on the main thread.
		 * @param connection client connection
		 * @param code the closing code
		 * @param reason the reason why connecting failed.
		 */
		virtual void onConnectionFailed(const WebSocketConnection& connection, int code, const std::string& reason) override;

		/**
		 * Called by web-socket server endpoint when a new message is received.
		 * Generates and forwards a nap::WebSocketMessageReceivedEvent to the running application on the main thread.
		 * @param connection the client connection.
		 * @param message the message received from the client.
		 */
		virtual void onMessageReceived(const WebSocketConnection& connection, const WebSocketMessage& message) override;
	};

	// Object creator used for constructing the web-socket server
	using WebSocketServerObjectCreator = rtti::ObjectCreator<WebSocketServer, WebSocketService>;
}
