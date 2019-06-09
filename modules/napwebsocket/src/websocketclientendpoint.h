#pragma once

// Local Includes
#include "websocketutils.h"
#include "websocketconnection.h"
#include "websocketmessage.h"

// External Includes
#include <nap/device.h>
#include <memory.h>
#include <future>
#include <nap/signalslot.h>
#include <mutex>

namespace nap
{
	class IWebSocketClient;

	/**
	 * Web-socket client endpoint.
	 */
	class NAPAPI WebSocketClientEndPoint : public Device
	{
		friend class IWebSocketClient;
		RTTI_ENABLE(Device)
	public:
		virtual ~WebSocketClientEndPoint();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		virtual void stop() override;

		virtual bool start(nap::utility::ErrorState& error) override;

		/**
		 * Sends a message to the specified connection
		 * @param connection the server connection
		 * @param message the message to send
		 * @param code message type
		 * @param error contains the error if sending fails
		 * @return if message was send successfully
		 */
		bool send(const WebSocketConnection& connection, const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error);

		/**
		 * Sends a message using the given payload and opcode to the specified connection
		 * @param connection the server connection
		 * @param payload the message buffer
		 * @param length total number of bytes
		 * @param code message type
		 * @param error contains the error if sending fails
		 * @return if message was send successfully
		 */
		bool send(const WebSocketConnection& connection, void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error);


		bool mAllowFailure = true;												///< Property: 'AllowFailure' if the client connection to the server is allowed to fail on start
		bool mLogConnectionUpdates = true;										///< Property: "LogConnectionUpdates" if client / server connection information is logged to the console.
		EWebSocketLogLevel mLibraryLogLevel = EWebSocketLogLevel::Warning;		///< Property: "LibraryLogLevel" library related equal to or higher than requested are logged.

	private:
		uint32 mLogLevel = 0;													///< Converted library log level
		uint32 mAccessLogLevel = 0;												///< Log client / server connection data
		bool mRunning = false;													///< If the client connection to the server is open						
		wspp::ClientEndPoint mEndPoint;											///< Websocketpp client end point
		std::vector<IWebSocketClient*> mClients;								///< Websocketpp client connections
		std::future<void> mClientTask;											///< The client server thread
		std::mutex mConnectionMutex;											///< Ensures connectivity thread safety

		/**
		 * Runs the end point in a background thread until stopped.
		 */
		void run();

		/**
		 * Connects a client to a server. The connection is managed by this endpoint.
		 * @param uri server uri.
		 * @return the newly created web-socket connection
		 */
		bool registerClient(IWebSocketClient& client , utility::ErrorState& error);

		/**
		 * Closes all active connections
		 */
		bool closeAll(utility::ErrorState& error);

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
		void onMessageReceived(wspp::ConnectionHandle con, wspp::MessagePtr msg);

		// THIS IS AN INIT WORKAROUND: TODO: FIX ORDER OF DESTRUCTION
		void onClientDestroyed(const IWebSocketClient& client);
		nap::Slot<const IWebSocketClient&> mClientDestroyed = { this, &WebSocketClientEndPoint::onClientDestroyed };

		IWebSocketClient* findClient(wspp::ConnectionPtr connection);
		void removeClient(const IWebSocketClient& client);
	};
}
