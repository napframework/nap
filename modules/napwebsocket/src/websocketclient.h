#pragma once

// Local Includes
#include "websocketclientendpoint.h"
#include "websocketinterface.h"

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <nap/signalslot.h>
#include <rtti/factory.h>
#include <atomic>

namespace nap
{
	/**
	 * Interface for all web-socket clients.
	 * Derive from this class to implement your own web-socket client.
	 * Override the various virtual functions to receive web-socket client updates.
	 * The virtual functions are called from a different thread than the main thread.
	 * It is your responsibility to ensure thread-safety.
	 * On initialization the client registers itself with a nap::WebSocketClientEndPoint.
	 */
	class NAPAPI IWebSocketClient : public WebSocketInterface
	{
		friend class WebSocketClientEndPoint;
		friend class WebSocketClientWrapper;

		RTTI_ENABLE(WebSocketInterface)
	public:
		/**
		 * Constructor
		 * @param service the web-socket service that forwards events to the application.
		 */
		IWebSocketClient(WebSocketService& service);

		// Destructor
		virtual ~IWebSocketClient();

		/**
		* Registers the web-socket client interface with the endpoint.
		* @param errorState contains the error message when initialization fails
		* @return if initialization succeeded.
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return if the client is connected
		 */
		bool isOpen() const;

		/**
		 * @return server connection handle.
		 */
		const WebSocketConnection& getConnection() const				{ return mConnection; }

		ResourcePtr<WebSocketClientEndPoint> mEndPoint;					///< Property: 'EndPoint' the client endpoint that manages all connections.
		std::string mURI;												///< Property: "UIR" Server URI to open connection to.

	protected:
		WebSocketConnection mConnection;								///< Web-socket connection

		/**
		 * Occurs when a new connection to the server is opened.
		 */
		virtual void onConnectionOpened() = 0;

		/**
		 * Occurs when the connection to the server is closed.
		 * @param code the closing code
		 * @param reason the reason for closing the connection.
		 */
		virtual void onConnectionClosed(int code, const std::string& reason) = 0;

		/**
		 * Occurs when connecting to the server fails.
		 * @param code the closing code
		 * @param reason the reason why the connection failed.
		 */
		virtual void onConnectionFailed(int code, const std::string& reason) = 0;

		/**
		 * Occurs when a new message from the server to this client is received.
		 * @param msg the received message
		 */
		virtual void onMessageReceived(const WebSocketMessage& msg);

	private:
		// Called by web-socket client endpoint when the connection is opened
		void connectionOpened();
		// Called by web-socket client endpoint when the connection is closed
		void connectionClosed(int code, const std::string& reason);
		// Called by web-socket client endpoint when the connection failed to establish
		void connectionFailed(int code, const std::string& reason);
		// Called by web-socket client endpoint when a new message is received
		void messageReceived(const WebSocketMessage& msg);

		std::atomic<bool> mOpen = { false };				///< If this client is currently connected
		nap::Signal<const IWebSocketClient&> destroyed;		///< Called when the client is destroyed
	};


	//////////////////////////////////////////////////////////////////////////
	// WebSocketClient
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Default implementation of a web-socket client.
	 * Converts connection updates and messages into web-socket events. 
	 * These events are consumed by the nap::WebSocketService on the main application thread.
	 * On update all events in the queue are forwarded to a nap::WebSocketComponent.
	 * Use a nap::WebSocketComponent to receive and react to web-socket events in your application.
	 */
	class NAPAPI WebSocketClient : public IWebSocketClient
	{
		RTTI_ENABLE(IWebSocketClient)
	public:
		/**
		 * Constructor
		 * @param service the web-socket service that forwards events to the application.
		 */
		WebSocketClient(WebSocketService& service);

		// Destructor
		virtual ~WebSocketClient();

		/**
		 * Initialize this object after de-serialization.
		 * Registers the web-socket client with the endpoint.
		 * @param errorState contains the error message when initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Sends a message with the given opcode to the server. The message is send immediately. 
		 * @param message the message to send
		 * @param code message type
		 * @param error contains the error if sending fails
		 * @return if the message was sent successfully
		 */
		bool send(const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error);

		/**
		 * Sends a message using the given payload and opcode to the server.
		 * The message is send immediately. 
		 * @param payload the message buffer
		 * @param length total number of bytes
		 * @param code message type
		 * @param error contains the error if sending fails
		 * @return if the message was sent successfully
		 */
		bool send(void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error);

		/**
		 * Sends a message to the server. The message is send immediately. 
		 * @param message the message to send
		 * @param error contains the error if sending fails
		 * @return if the message was sent successfully.
		 */
		bool send(const WebSocketMessage& message, nap::utility::ErrorState& error);

	protected:
		/**
		 * Called by web-socket client endpoint when the connection is opened.
		 * Generates and forwards a nap::WebSocketConnectionEvent to the running application on the main thread.
		 */
		virtual void onConnectionOpened() override;

		/**
		 * Called by web-socket client endpoint when the connection is closed.
		 * Generates and forwards a nap::WebSocketConnectionClosedEvent to the running application on the main thread.
		 * @param code the closing code
		 * @param reason the reason why the connection was closed.
		 */
		virtual void onConnectionClosed(int code, const std::string& reason) override;
		
		/**
		 * Called by web-socket client endpoint when the connection failed to establish.
		 * Generates and forwards a nap::WebSocketConnectionFailedEvent to the running application on the main thread.
		 * @param code the closing code
		 * @param reason the reason why connecting failed.
		 */
		virtual void onConnectionFailed(int code, const std::string& reason) override;
		
		/**
		 * Called by web-socket client endpoint when a new message is received.
		 * Generates and forwards a nap::WebSocketMessageReceivedEvent to the running application on the main thread.
		 * @param code the closing code
		 * @param reason the reason why connecting failed.
		 */
		virtual void onMessageReceived(const WebSocketMessage& msg) override;
	};

	// Object creator used for constructing the web-socket client
	using WebSocketClientObjectCreator = rtti::ObjectCreator<WebSocketClient, WebSocketService>;
}
