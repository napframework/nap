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
    template<typename config>
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
	class NAPAPI WebSocketClientEndPointBase : public Device
	{
		friend class IWebSocketClient;
		RTTI_ENABLE(Device)
	public:

		/**
		 * Sends a message to a server.
		 * @param connection the client connection to the server.
		 * @param message the message to send.
		 * @param code type of message.
		 * @param error contains the error if sending fails.
		 * @return if message was sent successfully
		 */
		virtual bool send(const WebSocketConnection& connection, const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error) = 0;

		/**
		 * Sends a message using the given payload and opcode to a server.
		 * @param connection the client connection to the server.
		 * @param payload the message buffer.
		 * @param length buffer size in bytes.
		 * @param code type of message.
		 * @param error contains the error if sending fails.
		 * @return if message was sent successfully.
		 */
		virtual bool send(const WebSocketConnection& connection, void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error) = 0;

		bool mLogConnectionUpdates = true;										///< Property: "LogConnectionUpdates" if client / server connection information is logged to the console.
		EWebSocketLogLevel mLibraryLogLevel = EWebSocketLogLevel::Warning;		///< Property: "LibraryLogLevel" library related equal to or higher than requested are logged.

	protected:

		/**
		 * Connects a nap client to a server. The new connection is managed by this endpoint.
		 * The client is added to the list of internally managed clients.
		 * @param client the client to register
		 * @param error contains the error is registration fails.
		 * @return if the client was registered successfully
		 */
		virtual bool registerClient(IWebSocketClient& client, utility::ErrorState& error) = 0;

		/**
		 * Removes a client (resource) from the list of actively managed connection.
		 * If the client connection is currently open it will be closed.
		 * Asserts if the client isn't part of the system or can't be removed.
		 * @param client the client to remove.
		 */
		virtual void unregisterClient(const IWebSocketClient& client) = 0;
	};

    template<typename config>
    class NAPAPI WebSocketClientEndPoint : public WebSocketClientEndPointBase
    {
    RTTI_ENABLE(WebSocketClientEndPointBase)
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
        websocketpp::client<config> mEndPoint; ///< websocketpp client end point
    };

    using WebSocketClientEndPointNoTLS = WebSocketClientEndPoint<wspp::Config>;

    class WebSocketClientEndPointTLS : public WebSocketClientEndPoint<wspp::ConfigTLS>
    {
    RTTI_ENABLE(WebSocketClientEndPoint<wspp::ConfigTLS>)
    public:
        /**
         * Starts the endpoint. This is a non-blocking call.
         * New connections are accepted. Connection updates and messages are received in a background thread.
         * @param error contains the error if the endpoint can't be started.
         * @return if the endpoint started.
         */
        bool start(nap::utility::ErrorState &error) override;

        std::string mCertificateChainFile = ""; ///< Property: "CertificateChainFile" path to the certificate chain file
        std::string mHostName = ""; ///< Property: "HostName" host name to verify against the certificate
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
	class NAPAPI WebSocketClientWrapper final
	{
        template<typename X>
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
		WebSocketClientWrapper(IWebSocketClient& client,
                               websocketpp::client<config>& endPoint,
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

    template<typename config>
    WebSocketClientWrapper<config>::~WebSocketClientWrapper()
    {
        // Not disconnected by server or client
        assert(!mOpen);
        mResource = nullptr;
        mEndPoint = nullptr;
    }
}
