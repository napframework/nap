/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
#include <unordered_map>

namespace nap
{
	class IWebSocketServer;

	/**
	 * Server endpoint role. Manages all client connections.
	 *
	 * On start the web-socket endpoint starts listening to connection requests, updates and messages on a background thread.
	 * The endpoint is a device that can be started and stopped. When stopped all
	 * active client-server connections are closed. This occurs when file changes are detected
	 * and the content of the application is hot-reloaded. A call to open is non blocking. 
	 * Messages are forwarded to all clients that implement the nap::IWebSocketClient interface.
	 * Clients must register themselves to the various signals to receive connection updates and messages.
	 * Right now SSL encryption is NOT supported. This will change in the future. Note that depending on your operating 
	 * system you might have to run the application as administrator to open a web-socket.
	 *
	 * By default the server accepts all client connection requests. Change the 'AccessMode' property to 'Ticket'
	 * or 'Reserved' to add a client identification scheme. When set to 'Ticket' the server accepts
	 * every client connection with a ticket. When set to 'Reserved' the server only accepts the connection
	 * if it has a matching ticket. Tickets that are accepted by the server can be added to the 'Clients' property.
	 * 
	 * A good analogy is that of attending a concert. When the mode is set to 'Everyone' you don't need to 
	 * pick up a ticket in order to enter the venue. You can just walk in. When the mode is set to 'Ticket' 
	 * you need to buy a ticket and show it at the front door. Everyone can buy a ticket. 
	 * When the mode is set to 'Reserved' you need to be on the guest list.
	 * If you're not on the guest-list you are not allowed to buy a ticket, let alone enter the venue.
	 *
	 * For both modes (Ticket and Reserved) you need to acquire a ticket by sending a HTTP 'POST' request 
	 * to the server. The body of the post should contain a JSON formatted string that contains 2 fields:
	 * 'user' and 'pass'. Both fields are always required. For example:
	 * ~~~~~
	 *	{
	 *		"user": "napuser"		///< Always necessary
	 *		"pass": "letmein"		///< Always necessary
	 *	}
	 * ~~~~~
	 *
	 * The server generates a new ticket based on the provided information and sends it back to the client that made the request.
	 * The received ticket should be specified as the first SUB-PROTOCOL argument when creating the web-socket. The server extracts the ticket 
	 * on authorization and checks if it is valid. After validation the connection is accepted or rejected. Note that when the
	 * mode is set to 'Everyone' the server will not serve any tickets and you should not provide the system with one when 
	 * connecting to the server. In that case: connect without specifying a sub-protocol.
	 * 
	 * The reason for having the 'Ticket' validation mode (next to 'Reserved') is to prevent any user or bot to log in 
	 * automatically but allow 'interested' users free access. Every nap::WebSocketClient can make it's own ticket and 
	 * therefore doesn't require the http post request. The NAP client still needs to have a correct username and password 
	 * if required by the server.
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
		 * @param errorState contains the error when initialization fails
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
		 * Register a server for this endpoint so that it receives notifications from the endpoint.
		 */
		void registerListener(IWebSocketServer& server);

		/**
		 * Unregister a server for this endpoint so that it stops receiving notifications from the endpoint.
		 */
		void unregisterListener(IWebSocketServer& server);

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

		/**
		 * @param message the message to send
		 * @param code message type
		 * @param error contains the error if sending fails
		 * @param message the message
		 */
		bool broadcast(const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error);

		/**
		 * Broadcasts a message to all connected clients using the given payload and opcode.
		 * @param payload the message buffer
		 * @param length size of the buffer in bytes
		 * @param code message type
		 * @param error contains the error if sending fails
		 * @return if message was send successfully
		 */
		bool broadcast(void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error);

		/**
		 * @return a client connection host-name, empty string if the connection is not managed by this end-point.
		 */
		std::string getHostName(const WebSocketConnection& connection);

		/**
		 * Returns a list of all currently connected client host-names.
		 * @param outHosts the list of connected client host-names.
		 */
		void getHostNames(std::vector<std::string>& outHosts);

		/**
		 * @return total number of active client connections
		 */
		int getConnectionCount();

		/**
		 * @return if the server end point accepts new connections
		 */
		bool acceptsNewConnections();

		EAccessMode mMode = EAccessMode::EveryOne;							///< Property: "AccessMode" client connection access mode.
		int mConnectionLimit = -1;											///< Property: "ConnectionLimit" number of allowed client connections at once, -1 = no limit
		int mPort = 80;														///< Property: "Port" to open and listen to for client requests.
		bool mLogConnectionUpdates = true;									///< Property: "LogConnectionUpdates" if client / server connect information is logged to the console.
		bool mAllowPortReuse = false;										///< Property: "AllowPortReuse" if the server connection can be re-used by other processes.
		EWebSocketLogLevel mLibraryLogLevel = EWebSocketLogLevel::Warning;	///< Property: "LibraryLogLevel" library messages equal to or higher than requested are logged.
		std::vector<ResourcePtr<WebSocketTicket>> mClients;					///< Property: "Clients" All authorized clients when mode is set to 'Reserved'"
		std::string mAccessAllowControlOrigin = "*";						///< Property: "AllowControlOrigin" Access-Control-Allow-Origin response header value. Indicates if the server response can be shared with request code from the given origin.
		std::string	mIPAddress = "";										///< Property: 'IPAddress' this server IP Address, when left empty the first available ethernet adapter is chosen.

	private:
		std::mutex mListenerMutex;
		std::vector<IWebSocketServer*> mListeners;

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
		 * Creates and serves a ticket based on the current Access Mode.
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

		bool mRunning = false;													///< If the server is accepting and managing client connections.
		std::mutex mConnectionMutex;											///< Ensures connections are added / removed safely.
		std::unordered_set<WebSocketTicketHash> mClientHashes;					///< Accepted client ticket hashes
		wspp::ServerEndPoint mEndPoint;											///< The websocketpp server end-point
		uint32 mLogLevel = 0;													///< Converted library log level
		uint32 mAccessLogLevel = 0;												///< Log client / server connection data
		std::future<void> mServerTask;											///< The background server thread
		std::vector<wspp::ConnectionHandle> mConnections;						///< List of all low level connections
	};
}
