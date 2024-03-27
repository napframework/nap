/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "iwebsocketserverendpoint.h"
#include "websocketmessage.h"
#include "websocketserver.h"

// External Includes
#include <memory.h>
#include <future>
#include <mutex>
#include <unordered_set>
#include <mathutils.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/error/en.h>

namespace nap
{
    /**
	 * Web socket server endpoint implementation using a specific configuration.
	 * Manages all client connections, where the `config` is the websocketpp (secured / unsecured)
	 * end-point configuration type.
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
    template<typename config>
    class WebSocketServerEndPointSetup : public IWebSocketServerEndPoint
    {
		RTTI_ENABLE(IWebSocketServerEndPoint)
    public:
		// Stop endpoint on destruction
        ~WebSocketServerEndPointSetup() override;

        /**
         * Initializes the server endpoint.
         * @param errorState contains the error when initialization fails
         * @return if initialization succeeded
         */
        bool init(utility::ErrorState& errorState) override;

        /**
         * Opens the port and starts listening to connection requests,
         * connection updates and messages in a background thread.
         * @param error contains the error if the operation failed.
         * @return if the endpoint started successfully.
         */
        bool start(nap::utility::ErrorState& error) override;

        /**
         * @return if the current end point is open and running
         */
        bool isOpen() const override;

        /**
         * Stops the end-point from running, all active connections are closed.
         */
        void stop() override;

        /**
         * Sends a message to a client.
         * @param connection the client connection
         * @param message the message to send
         * @param code message type
         * @param error contains the error if sending fails
         * @return if message was send successfully
         */
        bool send(const WebSocketConnection& connection, const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error) override;

        /**
         * Sends a message to a client using the given payload and opcode.
         * @param connection the client connection
         * @param payload the message buffer
         * @param length size of the buffer in bytes
         * @param code message type
         * @param error contains the error if sending fails
         * @return if message was send successfully
         */
        bool send(const WebSocketConnection& connection, void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error) override;

        /**
         * Sends a message to a all connected clients.
         * @param message the message to send
         * @param code message type
         * @param error contains the error if sending fails
         * @param message the message
         */
        bool broadcast(const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error) override;

        /**
         * Broadcasts a message to all connected clients using the given payload and opcode.
         * @param payload the message buffer
         * @param length size of the buffer in bytes
         * @param code message type
         * @param error contains the error if sending fails
         * @return if message was send successfully
         */
        bool broadcast(void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error) override;

        /**
         * @return a client connection host-name, empty string if the connection is not managed by this end-point.
         */
        std::string getHostName(const WebSocketConnection& connection) override;

        /**
         * Returns a list of all currently connected client host-names.
         * @param outHosts the list of connected client host-names.
         */
        void getHostNames(std::vector<std::string>& outHosts) override;

		/**
		 * @return total number of active connections
		 */
        int getConnectionCount() override;

        /**
         * @return if the server end point accepts new connections
         */
         bool acceptsNewConnections() override;

    protected:
		/**
         * Register a server for this endpoint so that it receives notifications from the endpoint.
         * @param server the server to register
         */
        void registerListener(IWebSocketServer& server) override;

        /**
         * Unregister a server for this endpoint so that it stops receiving notifications from the endpoint.
         * @param server the server to unregister
         */
        void unregisterListener(IWebSocketServer& server) override;

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
        websocketpp::server<config> mEndPoint;									///< The websocketpp server end-point
        uint32 mLogLevel = 0;													///< Converted library log level
        uint32 mAccessLogLevel = 0;												///< Log client / server connection data
        std::future<void> mServerTask;											///< The background server thread
        std::vector<wspp::ConnectionHandle> mConnections;						///< List of all low level connections
		std::mutex mListenerMutex;												///< Ensures registration is thread safe
		std::vector<IWebSocketServer*> mListeners;								///< All registered web socket servers
    };


	// Not secured web-socket server end point
    using WebSocketServerEndPoint = WebSocketServerEndPointSetup<wspp::Config>;


    /**
     * Secured web-socket server end point connection.
     * Adds transport layer security (TLS) using the provided certificate and key.
     */
    class NAPAPI SecureWebSocketServerEndPoint : public WebSocketServerEndPointSetup<wspp::ConfigTLS>
    {
		RTTI_ENABLE(WebSocketServerEndPointSetup<wspp::ConfigTLS>)
    public:
		/**
		 * Transport layer security (TLS) mode.
		 * See https://wiki.mozilla.org/Security/Server_Side_TLS for more details about the TLS modes.
		 */
		enum class ETLSMode : nap::uint8
		{
			Intermediate = 0,		///< Recommended config for general-purpose server.
			Modern					///< For modern clients with no need for backwards compatibility.
		};

		ETLSMode mMode = ETLSMode::Intermediate;				///< Property: "Mode" TLS configuration mode
		std::string mCertificateFile;							///< Property: "CertificateFile" TLS certificate file (.pem)
        std::string mPrivateKeyFile;							///< Property: "PrivateKeyFile" TLS private key file (.private)
        std::string mPassphrase;								///< Property: "Passphrase" password for the private key, only required when key is generated with one.

        /**
         * Initializes the server endpoint.
         * @param errorState contains the error when initialization fails
         * @return if initialization succeeded
         */
        bool init(utility::ErrorState& errorState) override;

        /**
         * Opens the port and starts listening to connection requests,
         * connection updates and messages in a background thread.
         * @param error contains the error if the operation failed.
         * @return if the endpoint started successfully.
         */
        bool start(nap::utility::ErrorState &error) override;

    private:
        /**
         * Called when the TLS context is initialized.
		 * @param con connection handle
		 * @return shared pointer to ssl context
		 */
		std::shared_ptr<asio::ssl::context> onTLSInit(wspp::ConnectionHandle con);
	};


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename config>
	bool WebSocketServerEndPointSetup<config>::start(nap::utility::ErrorState& error)
	{
		// Run until stopped
		assert(!mRunning);

		// Listen to messages on this specific port and ip address (if given)
		std::error_code stdec;
		if (mIPAddress.empty())
		{
			mEndPoint.listen(static_cast<uint16>(mPort), stdec);
		}
		else
		{
			mEndPoint.listen(mIPAddress, utility::stringFormat("%d", static_cast<uint16>(mPort)), stdec);
		}

		// Contains the error when opening port fails.
		if (stdec)
		{
			error.fail(stdec.message());
			return false;
		}

		// Queues a connection accept operation
		mEndPoint.start_accept(stdec);
		if (stdec)
		{
			error.fail(stdec.message());
			return false;
		}

		// Run until stopped
		mServerTask = std::async(std::launch::async, std::bind(&WebSocketServerEndPointSetup<config>::run, this));
		mRunning = true;

		return true;
	}


	template<typename config>
	bool WebSocketServerEndPointSetup<config>::isOpen() const
	{
		return mRunning;
	}


	template<typename config>
	bool WebSocketServerEndPointSetup<config>::send(const WebSocketConnection& connection, const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error)
	{
		std::error_code stdec;
		mEndPoint.send(connection.mConnection, message, static_cast<wspp::OpCode>(code), stdec);
		if (stdec)
		{
			error.fail(stdec.message());
			return false;
		}
		return true;
	}


	template<typename config>
	bool WebSocketServerEndPointSetup<config>::send(const WebSocketConnection& connection, void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error)
	{
		std::error_code stdec;
		mEndPoint.send(connection.mConnection, payload, length, static_cast<wspp::OpCode>(code), stdec);
		if (stdec)
		{
			error.fail(stdec.message());
			return false;
		}
		return true;
	}


	template<typename config>
	bool WebSocketServerEndPointSetup<config>::broadcast(const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error)
	{
		bool success(true);
		std::lock_guard<std::mutex> lock(mConnectionMutex);
		for (auto& connection : mConnections)
		{
			if (!send(connection, message, code, error))
				success = false;
		}
		return success;
	}


	template<typename config>
	bool WebSocketServerEndPointSetup<config>::broadcast(void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error)
	{
		bool success(true);
		std::lock_guard<std::mutex> lock(mConnectionMutex);
		for (auto& connection : mConnections)
		{
			if (!send(connection, payload, length, code, error))
				success = false;
		}
		return success;
	}


	template<typename config>
	std::string WebSocketServerEndPointSetup<config>::getHostName(const WebSocketConnection& connection)
	{
		std::error_code stdec;
		auto cptr = mEndPoint.get_con_from_hdl(connection.mConnection, stdec);
		return stdec ? "" : cptr->get_host();
	}

	template<typename config>
	void WebSocketServerEndPointSetup<config>::getHostNames(std::vector<std::string>& outHosts)
	{
		outHosts.clear();
		std::error_code stdec;
		std::lock_guard<std::mutex> lock(mConnectionMutex);
		outHosts.reserve(mConnections.size());
		for (auto& connection : mConnections)
		{
			auto cptr = mEndPoint.get_con_from_hdl(connection, stdec);
			if (!stdec)
				outHosts.emplace_back(cptr->get_host());
		}
	}


	template<typename config>
	int WebSocketServerEndPointSetup<config>::getConnectionCount()
	{
		std::lock_guard<std::mutex> lock(mConnectionMutex);
		return mConnections.size();
	}

	template<typename config>
	bool WebSocketServerEndPointSetup<config>::acceptsNewConnections()
	{
		if (mConnectionLimit < 0)
			return true;

		std::lock_guard<std::mutex> lock(mConnectionMutex);
		return mConnections.size() < mConnectionLimit;
	}


	template<typename config>
	void WebSocketServerEndPointSetup<config>::run()
	{
		// Start running until stopped
		mEndPoint.run();
	}


	template<typename config>
	void WebSocketServerEndPointSetup<config>::stop()
	{
		if (mRunning)
		{
			// Stop listening for new connections
			std::error_code stdec;
			mEndPoint.stop_listening(stdec);
			if (stdec)
			{
				assert(false);
				nap::Logger::error("%s: %s", mID.c_str(), stdec.message().c_str());
			}

			// Close all client connections
			utility::ErrorState napec;
			if (!disconnect(napec))
			{
				assert(false);
				nap::Logger::error("%s: %s", mID.c_str(), napec.toString().c_str());
			}

			// Explicitly stop
			mEndPoint.stop();

			// Wait for thread to finish
			assert(mServerTask.valid());
			mServerTask.wait();
			mRunning = false;
		}
	}


	template<typename config>
	WebSocketServerEndPointSetup<config>::~WebSocketServerEndPointSetup()
	{
		stop();
	}


	template<typename config>
	bool WebSocketServerEndPointSetup<config>::init(utility::ErrorState& errorState)
	{
		// Convert log levels
		mLogLevel = computeWebSocketLogLevel(mLibraryLogLevel);
		mAccessLogLevel = mLogConnectionUpdates ? websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload
			: websocketpp::log::alevel::fail;

		// Initiate logging
		mEndPoint.clear_error_channels(websocketpp::log::elevel::all);
		mEndPoint.set_error_channels(mLogLevel);

		mEndPoint.clear_access_channels(websocketpp::log::alevel::all);
		mEndPoint.set_access_channels(mAccessLogLevel);

		// If the endpoint can be re-used by other processes
		mEndPoint.set_reuse_addr(mAllowPortReuse);

		// Init asio
		std::error_code stdec;
		mEndPoint.init_asio(stdec);
		if (stdec)
		{
			errorState.fail(stdec.message());
			return false;
		}

		// Install connection open / closed handlers
		mEndPoint.set_http_handler(std::bind(&WebSocketServerEndPointSetup<config>::onHTTP, this, std::placeholders::_1));
		mEndPoint.set_open_handler(std::bind(&WebSocketServerEndPointSetup<config>::onConnectionOpened, this, std::placeholders::_1));
		mEndPoint.set_close_handler(std::bind(&WebSocketServerEndPointSetup<config>::onConnectionClosed, this, std::placeholders::_1));
		mEndPoint.set_fail_handler(std::bind(&WebSocketServerEndPointSetup<config>::onConnectionFailed, this, std::placeholders::_1));
		mEndPoint.set_validate_handler(std::bind(&WebSocketServerEndPointSetup<config>::onValidate, this, std::placeholders::_1));
		mEndPoint.set_ping_handler(std::bind(&WebSocketServerEndPointSetup<config>::onPing, this, std::placeholders::_1, std::placeholders::_2));

		// Install message handler
		mEndPoint.set_message_handler(std::bind(
			&WebSocketServerEndPointSetup<config>::onMessageReceived, this,
			std::placeholders::_1, std::placeholders::_2
		));

		// Create unique hashes out of tickets
		// Server side tickets are required to have a password and username
		for (const auto& ticket : mClients)
			mClientHashes.emplace(ticket->toHash());
		return true;
	}


	template<typename config>
	void WebSocketServerEndPointSetup<config>::onConnectionOpened(wspp::ConnectionHandle connection)
	{
		std::error_code stdec;
		auto cptr = mEndPoint.get_con_from_hdl(connection, stdec);
		if (stdec)
		{
			nap::Logger::error(stdec.message());
			return;
		}

		// Add to list of actively managed connections
		{
			std::lock_guard<std::mutex> lock(mConnectionMutex);
			mConnections.emplace_back(cptr);
		}

		for (IWebSocketServer* listener : mListeners)
			listener->onConnectionOpened(WebSocketConnection(connection));
	}


	template<typename config>
	void WebSocketServerEndPointSetup<config>::onConnectionClosed(wspp::ConnectionHandle connection)
	{
		std::error_code stdec;
		auto cptr = mEndPoint.get_con_from_hdl(connection, stdec);
		if (stdec)
		{
			nap::Logger::error(stdec.message());
			return;
		}

		// Signal that it closed
		for (IWebSocketServer* listener : mListeners)
			listener->onConnectionClosed(WebSocketConnection(connection), cptr->get_ec().value(), cptr->get_ec().message());

		// Remove from internal list of connections
		{
			std::lock_guard<std::mutex> lock(mConnectionMutex);
			auto found_it = std::find_if(mConnections.begin(), mConnections.end(), [&](const auto& it)
				{
					auto client_ptr = mEndPoint.get_con_from_hdl(it, stdec);
					if (stdec)
					{
						nap::Logger::error(stdec.message());
						return false;
					}
					return cptr == client_ptr;
				});

			// Having no connection to remove is a serious error and should never occur.
			if (found_it == mConnections.end())
			{
				assert(false);
				return;
			}
			mConnections.erase(found_it);
		}
	}


	template<typename config>
	void WebSocketServerEndPointSetup<config>::onConnectionFailed(wspp::ConnectionHandle connection)
	{
		std::error_code stdec;
		auto cptr = mEndPoint.get_con_from_hdl(connection, stdec);
		if (stdec)
		{
			nap::Logger::error(stdec.message());
			return;
		}

		for (IWebSocketServer* listener : mListeners)
			listener->onConnectionFailed(WebSocketConnection(connection), cptr->get_ec().value(), cptr->get_ec().message());
	}


	template<typename config>
	void WebSocketServerEndPointSetup<config>::onMessageReceived(wspp::ConnectionHandle con, wspp::MessagePtr msg)
	{
		WebSocketMessage message(msg);
		for (IWebSocketServer* listener : mListeners)
			listener->onMessageReceived(WebSocketConnection(con), message);
	}


	template<typename config>
	void WebSocketServerEndPointSetup<config>::onHTTP(wspp::ConnectionHandle con)
	{
		// Get handle to connection
		std::error_code stdec;
		auto conp = mEndPoint.get_con_from_hdl(con, stdec);
		if (stdec)
		{
			nap::Logger::error(stdec.message());
			conp->set_status(websocketpp::http::status_code::internal_server_error);
			return;
		}

		// Set whether the response can be shared with requesting code from the given origin.
		conp->append_header("Access-Control-Allow-Origin", mAccessAllowControlOrigin);

		// Get request method
		std::string method = conp->get_request().get_method();

		// Handle CORS preflight request
		if (method.compare("OPTIONS") == 0)
		{
			conp->set_status(websocketpp::http::status_code::no_content);
			conp->append_header("Access-Control-Allow-Methods", "OPTIONS, POST");
			conp->append_header("Access-Control-Allow-Headers", "Content-Type");
			return;
		}

		// Reject methods other than OPTIONS and POST
		if (method.compare("POST") != 0)
		{
			conp->set_status(websocketpp::http::status_code::method_not_allowed,
				"only OPTIONS and POST requests are allowed");
			return;
		}

		// When there is no access policy the server doesn't generate tickets
		if (mMode == EAccessMode::EveryOne)
		{
			conp->set_status(websocketpp::http::status_code::conflict,
				"unable to generate ticket, no access policy set");
			return;
		}

		// Get request body
		std::string body = conp->get_request_body();

		// Try to parse as JSON
		rapidjson::Document document;
		rapidjson::ParseResult parse_result = document.Parse(body.c_str());
		if (!parse_result || !document.IsObject())
		{
			conp->set_status(websocketpp::http::status_code::bad_request,
				"unable to parse as JSON");
			return;
		}

		// Extract user information, this field is required
		if (!document.HasMember("user"))
		{
			conp->append_header("WWW-Authenticate", "NAPUserPass");
			conp->set_status(websocketpp::http::status_code::unauthorized,
				"missing required member: 'user");
			return;
		}

		// Extract pass information, this field is required
		if (!document.HasMember("pass"))
		{
			conp->append_header("WWW-Authenticate", "NAPUserPass");
			conp->set_status(websocketpp::http::status_code::unauthorized,
				"missing required member: 'pass");
			return;
		}

		// Create ticket
		nap::WebSocketTicket ticket;

		// Populate and initialize
		ticket.mID = math::generateUUID();
		ticket.mUsername = document["user"].GetString();
		ticket.mPassword = document["pass"].GetString();
		utility::ErrorState error;
		if (!ticket.init(error))
		{
			conp->append_header("WWW-Authenticate", "NAPUserPass");
			conp->set_status(websocketpp::http::status_code::unauthorized,
				utility::stringFormat("invalid username or password: %s", error.toString().c_str()));
			return;
		}

		// If we only allow specific clients we extract the password here.
		// The pass is used in combination with the username to create a validation hash.
		if (mMode == EAccessMode::Reserved)
		{
			// Locate ticket, if there is no tick that matches the request
			// The username or password is wrong and the request invalid
			if (mClientHashes.find(ticket.toHash()) == mClientHashes.end())
			{
				conp->append_header("WWW-Authenticate", "NAPUserPass");
				conp->set_status(websocketpp::http::status_code::unauthorized,
					"invalid username or password");
				return;
			}
		}

		// Convert ticket to binary blob
		// This ticket can now be used to validate the request
		std::string ticket_str;
		if (!ticket.toBinaryString(ticket_str, error))
		{
			nap::Logger::error(error.toString());
			conp->set_status(websocketpp::http::status_code::internal_server_error);
			return;
		}

		// Set ticket as body
		conp->set_body(ticket_str);
		conp->set_status(websocketpp::http::status_code::ok);
	}


	template<typename config>
	bool WebSocketServerEndPointSetup<config>::onValidate(wspp::ConnectionHandle con)
	{
		// Get connection handle
		std::error_code stdec;
		auto conp = mEndPoint.get_con_from_hdl(con, stdec);
		if (stdec)
		{
			nap::Logger::error(stdec.message());
			conp->set_status(websocketpp::http::status_code::internal_server_error);
			return false;
		}

		// Make sure we accept new connections
		if (!acceptsNewConnections())
		{
			conp->set_status(websocketpp::http::status_code::forbidden,
				"client connection count exceeded");
			return false;
		}

		// Get sub-protocol field
		const std::vector<std::string>& sub_protocol = conp->get_requested_subprotocols();

		// When every client connection is allowed no sub-protocol field must be defined.
		if (mMode == EAccessMode::EveryOne)
		{
			if (sub_protocol.empty())
				return true;

			// Sub-protocol requested but can't authenticate.
			conp->set_status(websocketpp::http::status_code::not_found, "invalid sub-protocol");
			return false;
		}

		// We have tickets and therefore specific clients we accept.
		// Use the sub_protocol to extract client information
		if (sub_protocol.empty())
		{
			conp->set_status(websocketpp::http::status_code::forbidden,
				"unable to extract ticket");
			return false;
		}

		// Extract ticket
		conp->select_subprotocol(sub_protocol[0]);

		// Convert from binary into string
		utility::ErrorState error;
		rtti::DeserializeResult result;
		WebSocketTicket* client_ticket = WebSocketTicket::fromBinaryString(sub_protocol[0], result, error);
		if (client_ticket == nullptr)
		{
			conp->set_status(websocketpp::http::status_code::forbidden,
				"first sub-protocol argument is not a valid ticket object");
			return false;
		}

		// Valid ticket was extracted, allowed access
		if (mMode == EAccessMode::Ticket)
			return true;

		// Locate ticket
		if (mClientHashes.find(client_ticket->toHash()) == mClientHashes.end())
		{
			conp->set_status(websocketpp::http::status_code::forbidden,
				"not a valid ticket");
			return false;
		}

		// Welcome!
		return true;
	}


	template<typename config>
	bool WebSocketServerEndPointSetup<config>::onPing(wspp::ConnectionHandle con, std::string msg)
	{
		return true;
	}


	template<typename config>
	bool WebSocketServerEndPointSetup<config>::disconnect(nap::utility::ErrorState& error)
	{
		std::lock_guard<std::mutex> lock(mConnectionMutex);
		bool success = true;
		for (auto& connection : mConnections)
		{
			std::error_code stdec;
			mEndPoint.close(connection, websocketpp::close::status::going_away, "disconnected", stdec);
			if (stdec)
			{
				error.fail(stdec.message());
				success = false;
			}
		}
		mConnections.clear();
		return success;
	}


	template<typename config>
	void WebSocketServerEndPointSetup<config>::registerListener(IWebSocketServer& server)
	{
		std::unique_lock<std::mutex> lock(mListenerMutex);
		mListeners.push_back(&server);
	}


	template<typename config>
	void WebSocketServerEndPointSetup<config>::unregisterListener(IWebSocketServer& server)
	{
		std::unique_lock<std::mutex> lock(mListenerMutex);
		mListeners.erase(std::remove(mListeners.begin(), mListeners.end(), &server), mListeners.end());
	}


}
