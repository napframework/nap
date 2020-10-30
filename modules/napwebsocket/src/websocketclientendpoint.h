/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "websocketutils.h"
#include "websocketconnection.h"
#include "websocketmessage.h"

// External Includes
#include <nap/device.h>
#include <atomic>
#include <nap/signalslot.h>

namespace nap
{
	// Forward Declares
	class IWebSocketClient;
	class WebSocketClientWrapper;

	/**
	 * Manages a list of client-server connections and acts as the main portal for the client to the server.
	 * Every web-socket client receives connection updates and messages from an endpoint.
	 * The endpoint is a device that can be started and stopped. 
	 *
	 * When stopped all active client-server connections are closed. This occurs when file changes are detected
	 * and the content of the application is hot-reloaded. Typically an application
	 * has only one endpoint. Multiple clients can reference the same endpoint.
	 * Every connection update and message is forwarded to the client from a background thread.
	 * To receive connection updates and messages a client must be dervied from nap::IWebSocketClient.
	 *
	 * Note that depending on your operating system you might have to run the application as administrator
	 * to open a web-socket.
	 */
	class NAPAPI WebSocketClientEndPoint : public Device
	{
		friend class IWebSocketClient;
		RTTI_ENABLE(Device)
	public:

		/**
		 * Initialize this object after de-serialization
		 * @param errorState contains the error message when initialization fails.
		 * @return if the endpoint initialized correctly.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Stops the endpoint. All active connections are closed.
		 */
		virtual void stop() override;

		/**
		 * Starts the endpoint. This is a non-blocking call.
		 * New connections are accepted. Connection updates and messages are received in a background thread.
		 * @param error contains the error if the endpoint can't be started.
		 * @return if the endpoint started.
		 */
		virtual bool start(nap::utility::ErrorState& error) override;

		/**
		 * Sends a message to a server.
		 * @param connection the client connection to the server.
		 * @param message the message to send.
		 * @param code type of message.
		 * @param error contains the error if sending fails.
		 * @return if message was sent successfully
		 */
		bool send(const WebSocketConnection& connection, const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error);

		/**
		 * Sends a message using the given payload and opcode to a server.
		 * @param connection the client connection to the server.
		 * @param payload the message buffer.
		 * @param length buffer size in bytes.
		 * @param code type of message.
		 * @param error contains the error if sending fails.
		 * @return if message was sent successfully.
		 */
		bool send(const WebSocketConnection& connection, void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error);

		bool mLogConnectionUpdates = true;										///< Property: "LogConnectionUpdates" if client / server connection information is logged to the console.
		EWebSocketLogLevel mLibraryLogLevel = EWebSocketLogLevel::Warning;		///< Property: "LibraryLogLevel" library related equal to or higher than requested are logged.

	private:
		uint32 mLogLevel = 0;													///< Converted library log level
		uint32 mAccessLogLevel = 0;												///< Log client / server connection data
		bool mRunning = false;													///< If the client connection to the server is open						
		wspp::ClientEndPoint mEndPoint;											///< websocketpp client end point
		std::future<void> mClientTask;											///< The client server thread
		std::vector<std::unique_ptr<WebSocketClientWrapper>> mClients;			///< All unique client connections

		/**
		 * Runs the endpoint in a background thread until stopped.
		 */
		void run();

		/**
		 * Connects a nap client to a server. The new connection is managed by this endpoint.
		 * The client is added to the list of internally managed clients.
		 * @param client the client to register
		 * @param error contains the error is registration fails.
		 * @return the newly created web-socket connection
		 */
		bool registerClient(IWebSocketClient& client, utility::ErrorState& error);

		/**
		 * Removes a client (resource) from the list of actively managed connection.
		 * If the client connection is currently open it will be closed.
		 * Asserts if the client isn't part of the system or can't be removed.
		 * @param client the client to remove.
		 */
		void unregisterClient(const IWebSocketClient& client);
	};


	//////////////////////////////////////////////////////////////////////////
	// WebSocketClientWrapper
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Used internally by the nap::WebSocketClientEndPoint. Binds a client (resource) to a server connection.
	 * Connections and messages are received from various threads. This objects
	 * ensures that new information is forwarded to the right client without locking any resources.
	 */
	class NAPAPI WebSocketClientWrapper final
	{
		friend class WebSocketClientEndPoint;
	public:
		// Destructor
		~WebSocketClientWrapper();

	private:
		/**
		 * Called when a new connection is made
		 * @param connection handle to the connection with the server
		 */
		void onConnectionOpened(wspp::ConnectionHandle connection);

		/**
		 * Called when a collection is closed
		 * @param connection handle to the connection with the server
		 */
		void onConnectionClosed(wspp::ConnectionHandle connection);

		/**
		 * Called when a failed connection attempt is made
		 * @param connection handle to the connection with the server
		 */
		void onConnectionFailed(wspp::ConnectionHandle connection);

		/**
		 * Called when a new message is received
		 * @param connection handle to the connection with the server
		 * @param msg pointer to the message
		 */
		void onMessageReceived(wspp::ConnectionHandle connection, wspp::MessagePtr msg);

		/**
		 * Only client end point is able to construct this object.
		 * @param client web-socket client resource
		 * @param endPoint websocketpp end point.
		 * @param connection websocketpp pointer to connection.
		 */
		WebSocketClientWrapper(IWebSocketClient& client, wspp::ClientEndPoint& endPoint, wspp::ConnectionPtr connection);

		/**
		 * Disconnects the web-socket client, ensuring no callbacks are triggered when
		 * the connection is closed.
		 * @param error contains the error if the operation fails.
		 * @return if the client disconnected correct.y.
		 */
		bool disconnect(nap::utility::ErrorState& error);

		IWebSocketClient* mResource = nullptr;
		wspp::ClientEndPoint* mEndPoint = nullptr;
		wspp::ConnectionHandle mHandle;
		std::atomic<bool> mOpen = { false };
	};
}
