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
	class IWebSocketClient;
	class WebSocketClientWrapper;

	/**
	 * Manages a list of client connections and acts as the main portal for the client to the server.
	 * Every web-socket client receives connection updates and messages from an endpoint.
	 * The endpoint is a device that can be started and stopped. When stopped all
	 * active client-server connections are closed. This occurs when file changes are detected
	 * and the content of the application is hot-reloaded. Typically an application
	 * has only one endpoint. Multiple clients can reference the same endpoint.
	 * Every connection update and message is forwarded to the client from a different thread.
	 * To receive connection updates and messages a client must be dervied from nap::IWebSocketClient.
	 */
	class NAPAPI WebSocketClientEndPoint : public Device
	{
		friend class IWebSocketClient;
		RTTI_ENABLE(Device)
	public:
		virtual ~WebSocketClientEndPoint();

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
		 * New connections are accepted. Connection updates and messages are received in a separate thread.
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
		 * Connects a client to a server. The connection is managed by this endpoint.
		 * @param uri server uri.
		 * @param error contains the error is registration fails.
		 * @return the newly created web-socket connection
		 */
		bool registerClient(IWebSocketClient& client, utility::ErrorState& error);

		/**
		 * Occurs when a client (resource) is about to be destroyed. 
		 * Closes the connection and removes all listeners.
		 * @param client that is destroyed.
		 */
		void onClientDestroyed(const IWebSocketClient& client);
		nap::Slot<const IWebSocketClient&> mClientDestroyed = { this, &WebSocketClientEndPoint::onClientDestroyed };

		/**
		 * Removes a client (resource) from the list of actively managed connection.
		 * Asserts if the client isn't part of the system or can't be removed.
		 * @param client the client to remove.
		 */
		void removeClient(const IWebSocketClient& client);
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
		 */
		void onConnectionOpened(wspp::ConnectionHandle connection);

		/**
		 * Called when a collection is closed
		 */
		void onConnectionClosed(wspp::ConnectionHandle connection);

		/**
		 * Called when a failed connection attempt is made
		 */
		void onConnectionFailed(wspp::ConnectionHandle connection);

		/**
		 * Called when a new message is received
		 */
		void onMessageReceived(wspp::ConnectionHandle connection, wspp::MessagePtr msg);

		/**
		 * Only client end point is able to construct this object.
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
