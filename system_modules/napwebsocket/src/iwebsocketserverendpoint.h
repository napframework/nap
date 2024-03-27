/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "websocketutils.h"
#include "websocketticket.h"
#include "websocketconnection.h"

// External Includes
#include <utility/errorstate.h>
#include <nap/device.h>
#include <nap/resourceptr.h>
#include <mutex>

namespace nap
{
	// Forward declares
	class IWebSocketServer;

	/**
	 * Server endpoint interface. Manages all client connections.
	 *
	 * On start the web-socket endpoint starts listening to connection requests, updates and messages on a background thread.
	 * The endpoint is a device that can be started and stopped. When stopped all
	 * active client-server connections are closed. This occurs when file changes are detected
	 * and the content of the application is hot-reloaded. A call to open is non blocking.
	 * Messages are forwarded to all clients that implement the nap::IWebSocketClient interface.
	 * Clients must register themselves to the various signals to receive connection updates and messages.
	 * Note that depending on your operating system you might have to run the application as administrator to open a web-socket.
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
	class NAPAPI IWebSocketServerEndPoint : public Device
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

		/**
		 * @return if the current end point is open and running
		 */
		virtual bool isOpen() const = 0;

		/**
		 * Sends a message to a client.
		 * @param connection the client connection
		 * @param message the message to send
		 * @param code message type
		 * @param error contains the error if sending fails
		 * @return if message was send successfully
		 */
        virtual bool send(const WebSocketConnection& connection, const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error) = 0;

		/**
		 * Sends a message to a client using the given payload and opcode.
		 * @param connection the client connection
		 * @param payload the message buffer
		 * @param length size of the buffer in bytes
		 * @param code message type
		 * @param error contains the error if sending fails
		 * @return if message was send successfully
		 */
        virtual bool send(const WebSocketConnection& connection, void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error) = 0;

		/**
		 * Sends a message to a all connected clients.
		 * @param message the message to send
		 * @param code message type
		 * @param error contains the error if sending fails
		 * @param message the message
		 */
        virtual bool broadcast(const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error) = 0;

		/**
		 * Broadcasts a message to all connected clients using the given payload and opcode.
		 * @param payload the message buffer
		 * @param length size of the buffer in bytes
		 * @param code message type
		 * @param error contains the error if sending fails
		 * @return if message was send successfully
		 */
        virtual bool broadcast(void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error) = 0;

		/**
		 * @return a client connection host-name, empty string if the connection is not managed by this end-point.
		 */
        virtual std::string getHostName(const WebSocketConnection& connection) = 0;

		/**
		 * Returns a list of all currently connected client host-names.
		 * @param outHosts the list of connected client host-names.
		 */
        virtual void getHostNames(std::vector<std::string>& outHosts) = 0;

		/**
		 * @return total number of active client connections
		 */
		virtual int getConnectionCount() = 0;

		/**
		 * @return if the server end point accepts new connections
		 */
		virtual bool acceptsNewConnections() = 0;

		EAccessMode mMode = EAccessMode::EveryOne;							///< Property: "AccessMode" client connection access mode.
        int mConnectionLimit = -1;											///< Property: "ConnectionLimit" number of allowed client connections at once, -1 = no limit
        int mPort = 80;														///< Property: "Port" to open and listen to for client requests.
        bool mLogConnectionUpdates = true;									///< Property: "LogConnectionUpdates" if client / server connect information is logged to the console.
        bool mAllowPortReuse = false;										///< Property: "AllowPortReuse" if the server connection can be re-used by other processes.
        EWebSocketLogLevel mLibraryLogLevel = EWebSocketLogLevel::Warning;	///< Property: "LibraryLogLevel" library messages equal to or higher than requested are logged.
        std::vector<ResourcePtr<WebSocketTicket>> mClients;					///< Property: "Clients" All authorized clients when mode is set to 'Reserved'"
        std::string mAccessAllowControlOrigin = "*";						///< Property: "AllowControlOrigin" Access-Control-Allow-Origin response header value. Indicates if the server response can be shared with request code from the given origin.
        std::string	mIPAddress = "";										///< Property: 'IPAddress' this server IP Address, when left empty the first available ethernet adapter is chosen.

	protected:
		/**
		 * Register a server for this endpoint so that it receives notifications from the endpoint.
		 * @param server server to register as listener
		 */
        void registerListener(IWebSocketServer& server);

		/**
		 * Unregister a server for this endpoint so that it stops receiving notifications from the endpoint.
		 * @param server the server to unregister as listener
		 */
        void unregisterListener(IWebSocketServer& server);

		std::mutex mListenerMutex;											///< Ensures registration is thread safe
		std::vector<IWebSocketServer*> mListeners;							///< All registered web socket servers
    };
}
