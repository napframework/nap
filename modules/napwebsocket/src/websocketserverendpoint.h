#pragma once

// Local Includes
#include "websocketutils.h"
#include "websocketconnection.h"
#include "websocketmessage.h"
#include "websocketticket.h"

// External Includes
#include <memory.h>
#include <future>
#include <utility/errorstate.h>
#include <nap/device.h>
#include <nap/signalslot.h>
#include <nap/resourceptr.h>
#include <mutex>
#include <unordered_set>

namespace nap
{
	class IWebSocketServer;

	/**
	 * Server endpoint role. Manages all client connections.
	 * On start the web-socket endpoint starts listening to connection requests, updates and messages on a background thread.
	 * The endpoint is a device that can be started and stopped. When stopped all
	 * active client-server connections are closed. This occurs when file changes are detected
	 * and the content of the application is hot-reloaded. A call to open is non blocking. 
	 * Messages are forwarded to all clients that implement the nap::IWebSocketClient interface.
	 * Clients must register themselves to the various signals to receive connection updates and messages.
	 * Right now SSL encryption is NOT supported and all client connections are accepted. This will change in the future,
	 * but for now there is no interface to reject or accept an incoming client connection request.
	 */
	class NAPAPI WebSocketServerEndPoint : public Device
	{
		friend class IWebSocketServer;
		RTTI_ENABLE(Device)
	public:

		/**
		 * Various client access modes
		 */
		enum class EAccessMode : int
		{
			EveryOne		= 0,			///< Every client connection is allowed
			Ticket			= 1,			///< Every client connection with a ticket is allowed
			Reserved		= 2				///< Only clients that have a matching ticket are allowed
		};

		// default constructor
		WebSocketServerEndPoint();

		/**
		 * Calls stop. Closes all active client connections. 
		 */
		virtual ~WebSocketServerEndPoint();

		/**
		 * Initializes the server endpoint. 
		 * @param error contains the error when initialization fails
		 * @return if initialization succeeded
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Opens the port and starts listening to connection requests,
		 * connection updates and messages in a background thread.
		 * @param error contains the error if the operation failed.
		 * @return if the endpoint started successfully.
		 */
		virtual bool start(nap::utility::ErrorState& error) override;

		/**
		 * @return if the current end point is open and running
		 */
		bool isOpen() const;

		/**
		 * Stops the end-point from running, all active connections are closed.
		 */
		virtual void stop() override;

		/**
		 * Sends a message to a client.
		 * @param connection the client connection
		 * @param message the message to send
		 * @param code message type
		 * @param error contains the error if sending fails
		 * @return if message was send successfully
		 */
		bool send(const WebSocketConnection& connection, const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error);

		/**
		 * Sends a message to a client using the given payload and opcode.
		 * @param connection the client connection
		 * @param payload the message buffer
		 * @param length size of the buffer in bytes
		 * @param code message type
		 * @param error contains the error if sending fails
		 * @return if message was send successfully
		 */
		bool send(const WebSocketConnection& connection, void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error);

		EAccessMode mMode = EAccessMode::EveryOne;								///< Property: "AccessMode" client connection access mode
		int mPort = 80;															///< Property: "Port" to open and listen to for client requests.
		bool mLogConnectionUpdates = true;										///< Property: "LogConnectionUpdates" if client / server connect information is logged to the console.
		bool mAllowPortReuse = false;											///< Property: "AllowPortReuse" if the server connection can be re-used by other processes.
		EWebSocketLogLevel mLibraryLogLevel = EWebSocketLogLevel::Warning;		///< Property: "LibraryLogLevel" library messages equal to or higher than requested are logged.
		std::vector<ResourcePtr<WebSocketTicket>> mClients;						///< Property: "All authorized clients when mode is set to 'Reserved'"

		// Triggered when a new client connection is opened. Including client web-socket connection.
		nap::Signal<const WebSocketConnection&> connectionOpened;
		
		// Triggered when a client connection closed. Including client web-socket connection, close code and reason.
		nap::Signal<const WebSocketConnection&, int, const std::string&> connectionClosed;

		// Triggered when a client connection failed to establish. Including client web-socket connection, failure code and reason.
		nap::Signal<const WebSocketConnection&, int, const std::string&> connectionFailed;

		// Triggered when a new message from a client is received. Including client web-socket connection and message.
		nap::Signal<const WebSocketConnection&, const WebSocketMessage&> messageReceived;

	private:
		wspp::ServerEndPoint mEndPoint;											///< The websocketpp server end-point
		uint32 mLogLevel = 0;													///< Converted library log level
		uint32 mAccessLogLevel = 0;												///< Log client / server connection data
		std::future<void> mServerTask;											///< The background server thread
		std::vector<wspp::ConnectionPtr> mConnections;							///< List of all active connections

		/**
		 * Runs the end point in a background thread until stopped.
		 */
		void run();

		/**
		 * Called when a new client connection opened.
		 */
		void onConnectionOpened(wspp::ConnectionHandle connection);

		/**
		 * Called when a client collection is closed
		 */
		void onConnectionClosed(wspp::ConnectionHandle connection);

		/**
		 * Called on a failed client connection attempt.
		 */
		void onConnectionFailed(wspp::ConnectionHandle connection);

		/**
		 * Called when a new client message is received.
		 */
		void onMessageReceived(wspp::ConnectionHandle con, wspp::MessagePtr msg);

		/**
		 * Called when a http request is made.
		 * At the moment this just prints the request, we don't act upon it.
		 */
		void onHTTP(wspp::ConnectionHandle con);

		/**
		 * Validates the incoming connection. For now all connections are accepted.
		 */
		bool onValidate(wspp::ConnectionHandle con);

		/**
		 * Called when the server receives a ping message. Automatically pongs back.
		 */
		bool onPing(wspp::ConnectionHandle con, std::string msg);

		/**
		 * Closes all active client connections.
		 */
		bool disconnect(nap::utility::ErrorState& error);

		bool mRunning = false;					///< If the server is accepting and managing client connections.
		std::mutex mConnectionMutex;			///< Ensures connections are added / removed safely.
		std::unordered_set<WebSocketTicketHash> mClientHashes;
	};
}
