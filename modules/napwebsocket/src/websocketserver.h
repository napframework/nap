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
	 * Interface for all web-socket servers.
	 * Derive from this class to implement your own web-socket server.
	 * On initialization the client registers itself with a nap::WebSocketClientEndPoint.
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

		// Called by web-socket server endpoint when a new message is received
		virtual void onMessageReceived(const WebSocketConnection& connection, const WebSocketMessage& message) = 0;

		// Called by web-socket server endpoint when a client connection opened
		virtual void onConnectionOpened(const WebSocketConnection& connection) = 0;

		// Called by web-socket server endpoint when a client connection closed
		virtual void onConnectionClosed(const WebSocketConnection& connection, int code, const std::string& reason) = 0;

		// Called by web-socket server endpoint when a client connection failed to establish
		virtual void onConnectionFailed(const WebSocketConnection& connection, int code, const std::string& reason) = 0;

		ResourcePtr<WebSocketServerEndPoint> mEndPoint;	///< Property: 'EndPoint' the server endpoint that manages all client connections
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
		 * Initializes the server.
		 * @param errorState contains the error if the server can't be initialized.
		 * @return if the server initialized correctly.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Destroys the server.
		 */
		virtual void onDestroy() override;

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
		 * Sends a message to a client.
		 * @param connection the client connection
		 * @param message the message to send
		 * @param error contains the error if sending fails
		 * @return if message was sent successfully.
		 */
		bool send(const WebSocketConnection& connection, const WebSocketMessage& message, nap::utility::ErrorState& error);

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
