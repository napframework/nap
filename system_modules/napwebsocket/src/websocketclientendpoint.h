/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "iwebsocketclientendpoint.h"
#include "websocketmessage.h"
#include "websocketclient.h"

// External Includes
#include <atomic>
#include <nap/logger.h>

namespace nap
{
	// Forward Declares
    template<typename config>
	class WebSocketClientWrapper;

	/**
	 * Web socket client endpoint implementation using a specific configuration,
	 * where `config` is the websocketpp (secured / unsecured) end-point configuration type.
	 *
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
	 */
    template<typename config>
    class WebSocketClientEndPointSetup : public IWebSocketClientEndPoint
    {
    RTTI_ENABLE(IWebSocketClientEndPoint)
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
        bool start(nap::utility::ErrorState& error) override;

        /**
         * Sends a message to a server.
         * @param connection the client connection to the server.
         * @param message the message to send.
         * @param code type of message.
         * @param error contains the error if sending fails.
         * @return if message was sent successfully
         */
        bool send(const WebSocketConnection& connection, const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error) override;

        /**
         * Sends a message using the given payload and opcode to a server.
         * @param connection the client connection to the server.
         * @param payload the message buffer.
         * @param length buffer size in bytes.
         * @param code type of message.
         * @param error contains the error if sending fails.
         * @return if message was sent successfully.
         */
        bool send(const WebSocketConnection& connection, void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error) override;

    protected:
        /**
         * Runs the endpoint in a background thread until stopped.
         */
        void run();

        /**
         * Connects a nap client to a server. The new connection is managed by this endpoint.
         * The client is added to the list of internally managed clients.
         * @param client the client to register
         * @param error contains the error is registration fails.
         * @return if the client was registered successfully
         */
        bool registerClient(IWebSocketClient& client, utility::ErrorState& error) override;

        /**
         * Removes a client (resource) from the list of actively managed connection.
         * If the client connection is currently open it will be closed.
         * Asserts if the client isn't part of the system or can't be removed.
         * @param client the client to remove.
         */
        void unregisterClient(const IWebSocketClient& client) override;

        uint32 mLogLevel = 0;													///< Converted library log level
        uint32 mAccessLogLevel = 0;												///< Log client / server connection data
        bool mRunning = false;													///< If the client connection to the server is open
        std::future<void> mClientTask;											///< The client server thread
        std::vector<std::unique_ptr<WebSocketClientWrapper<config>>> mClients;	///< All unique client connections
        websocketpp::client<config> mEndPoint;									///< websocketpp client end point
    };


	// Not secured web-socket client end point connection.
    using WebSocketClientEndPoint = WebSocketClientEndPointSetup<wspp::Config>;


	/**
	 * Secured client end point connection.
	 */
    class NAPAPI SecureWebSocketClientEndPoint : public WebSocketClientEndPointSetup<wspp::ConfigTLS>
    {
		RTTI_ENABLE(WebSocketClientEndPointSetup<wspp::ConfigTLS>)
    public:
        /**
         * Starts the endpoint. This is a non-blocking call.
         * New connections are accepted. Connection updates and messages are received in a background thread.
         * @param error contains the error if the endpoint can't be started.
         * @return if the endpoint started.
         */
        bool start(nap::utility::ErrorState &error) override;

        std::string mCertificateChainFile;									///< Property: "CertificateChainFile" path to the certificate chain file
        std::string mHostName;												///< Property: "HostName" host name to verify against the certificate

    private:
        /**
         * Called when a new connection is made
         * @param handle connection handle to the connection with the server
         * @return shared pointer to the context
         */
        std::shared_ptr<asio::ssl::context> onTLSInit(websocketpp::connection_hdl);

        bool verifyCertificate(const char * hostname, bool preverified, asio::ssl::verify_context& ctx);
    };


	//////////////////////////////////////////////////////////////////////////
	// WebSocketClientWrapper
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Used internally by the nap::WebSocketClientEndPoint. Binds a client (resource) to a server connection.
	 * Connections and messages are received from various threads. This objects
	 * ensures that new information is forwarded to the right client without locking any resources.
	 */
    template<typename config>
	class WebSocketClientWrapper final
	{
        template<typename X>
		friend class WebSocketClientEndPointSetup;
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
		WebSocketClientWrapper(IWebSocketClient& client, websocketpp::client<config>& endPoint,
			typename websocketpp::endpoint<websocketpp::connection<config>, config>::connection_ptr connection);

		/**
		 * Disconnects the web-socket client, ensuring no callbacks are triggered when
		 * the connection is closed.
		 * @param error contains the error if the operation fails.
		 * @return if the client disconnected correct.y.
		 */
		bool disconnect(nap::utility::ErrorState& error);

		IWebSocketClient* mResource = nullptr;
        websocketpp::client<config>* mEndPoint = nullptr;
		wspp::ConnectionHandle mHandle;
		std::atomic<bool> mOpen = { false };
	};


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename config>
	bool WebSocketClientEndPointSetup<config>::init(utility::ErrorState& errorState)
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

		// Init asio
		std::error_code stdec;
		mEndPoint.init_asio(stdec);
		if (stdec)
		{
			errorState.fail(stdec.message());
			return false;
		}

		return true;
	}

	template<typename config>
	void WebSocketClientEndPointSetup<config>::stop()
	{
		// At this state we need to have an open end point and client thread
		assert(mRunning);
		assert(mClientTask.valid());

		// Make sure the end point doesn't restart
		mEndPoint.stop_perpetual();

		// Wait until all still active clients exit clean
		for (auto& client : mClients)
		{
			utility::ErrorState error;
			if (!client->disconnect(error))
				nap::Logger::error(error.toString());
		}

		// Wait until all clients exited
		mClientTask.wait();
		mRunning = false;

		// Clear all clients
		mClients.clear();
	}


	template<typename config>
	bool WebSocketClientEndPointSetup<config>::start(utility::ErrorState& error)
	{
		// Ensure state
		assert(!mRunning);

		// Ensure connection exists when server disconnects
		mEndPoint.start_perpetual();

		// Run client in background
		mClientTask = std::async(std::launch::async, std::bind(&WebSocketClientEndPointSetup<config>::run, this));

		mRunning = true;

		return true;
	}


	template<typename config>
	bool WebSocketClientEndPointSetup<config>::send(const WebSocketConnection& connection, const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error)
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
	bool WebSocketClientEndPointSetup<config>::send(const WebSocketConnection& connection, void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error)
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
	void WebSocketClientEndPointSetup<config>::run()
	{
		// Start running until stopped
		mEndPoint.run();
	}


	template<typename config>
	bool WebSocketClientEndPointSetup<config>::registerClient(IWebSocketClient& client, utility::ErrorState& error)
	{
		// Get shared pointer to connection
		std::error_code stdec;
		auto client_connection = mEndPoint.get_connection(client.mURI, stdec);
		if (stdec)
		{
			error.fail(stdec.message());
			return false;
		}

		// Send identification information if present on client
		if (client.mTicket != nullptr)
		{
			// Convert ticket into binary string
			std::string binary_string;
			if (!client.mTicket->toBinaryString(binary_string, error))
				return false;

			// Add as sub-protocol identifier.
			client_connection->add_subprotocol(binary_string, stdec);
			if (stdec)
			{
				error.fail(stdec.message());
				return false;
			}
		}

		// Create meta client and add to the list of internally managed clients
		std::unique_ptr<WebSocketClientWrapper<config>> meta_client(new WebSocketClientWrapper<config>(client,
			mEndPoint,
			client_connection));
		mClients.emplace_back(std::move(meta_client));

		// Store connection handle in client
		client.mConnection = WebSocketConnection(client_connection->get_handle());

		// TRY to connect, this occurs on a background thread.
		// All connection related state changes are handled in the WebSocketClientWrapper.
		mEndPoint.connect(client_connection);

		return true;
	}


	template<typename config>
	void WebSocketClientEndPointSetup<config>::unregisterClient(const IWebSocketClient& client)
	{
		auto found_it = std::find_if(mClients.begin(), mClients.end(), [&](const auto& it)
			{
				return it->mResource == &client;
			});

		if (found_it == mClients.end())
		{
			return;
		}

		// Disconnect if connected previously
		utility::ErrorState error;
		if (!(*found_it)->disconnect(error))
		{
			nap::Logger::error(error.toString());
		}
		mClients.erase(found_it);
	}


	template<typename config>
	WebSocketClientWrapper<config>::WebSocketClientWrapper(IWebSocketClient& client, websocketpp::client<config>& endPoint, typename websocketpp::endpoint<websocketpp::connection<config>, config>::connection_ptr connection) :
		mResource(&client), mEndPoint(&endPoint), mHandle(connection->get_handle())
	{
		// Connect callbacks (occur on different thread)
		connection->set_open_handler(std::bind(&WebSocketClientWrapper::onConnectionOpened, this, std::placeholders::_1));
		connection->set_close_handler(std::bind(&WebSocketClientWrapper::onConnectionClosed, this, std::placeholders::_1));
		connection->set_fail_handler(std::bind(&WebSocketClientWrapper::onConnectionFailed, this, std::placeholders::_1));

		// Install message handler
		connection->set_message_handler(std::bind(
			&WebSocketClientWrapper::onMessageReceived, this,
			std::placeholders::_1, std::placeholders::_2));
	}


	template<typename config>
	void WebSocketClientWrapper<config>::onConnectionOpened(wspp::ConnectionHandle connection)
	{
		assert(mResource != nullptr);
		mResource->connectionOpened();
		mOpen = true;
	}


	template<typename config>
	void WebSocketClientWrapper<config>::onConnectionClosed(wspp::ConnectionHandle connection)
	{
		// Extract actual connection, must be valid at this point
		std::error_code stdec;
		auto cptr = mEndPoint->get_con_from_hdl(connection, stdec);
		assert(!stdec);

		assert(mResource != nullptr);
		mResource->connectionClosed(stdec.value(), stdec.message());
		mOpen = false;
	}


	template<typename config>
	void WebSocketClientWrapper<config>::onConnectionFailed(wspp::ConnectionHandle connection)
	{
		// Extract actual connection, must be valid at this point
		std::error_code stdec;
		auto cptr = mEndPoint->get_con_from_hdl(connection, stdec);
		assert(!stdec);

		assert(mResource != nullptr);
		mResource->connectionFailed(stdec.value(), stdec.message());
		mOpen = false;
	}


	template<typename config>
	void WebSocketClientWrapper<config>::onMessageReceived(wspp::ConnectionHandle connection, wspp::MessagePtr msg)
	{
		assert(mResource != nullptr);
		mResource->messageReceived(WebSocketMessage(msg));
	}


	template<typename config>
	bool WebSocketClientWrapper<config>::disconnect(nap::utility::ErrorState& error)
	{
		if (mOpen)
		{
			// Get connection from handle, should be open by this point
			std::error_code stdec;
			auto cptr = mEndPoint->get_con_from_hdl(mHandle, stdec);
			if (stdec)
			{
				assert(false);
				error.fail(stdec.message());
				return false;
			}

			// Remove callbacks
			cptr->set_open_handler(nullptr);
			cptr->set_close_handler(nullptr);
			cptr->set_fail_handler(nullptr);
			cptr->set_message_handler(nullptr);

			// Now close
			mEndPoint->close(mHandle, websocketpp::close::status::going_away, "disconnected", stdec);
			if (stdec)
			{
				nap::Logger::error(stdec.message());
			}
			mOpen = false;
		}
		return true;
	}

	template<typename config>
    WebSocketClientWrapper<config>::~WebSocketClientWrapper()
    {
        // Not disconnected by server or client
        assert(!mOpen);
        mResource = nullptr;
        mEndPoint = nullptr;
    }
}
