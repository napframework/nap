/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "websocketclientendpoint.h"
#include "websocketclient.h"
#include "websocketmessage.h"

// External Includes
#include <nap/logger.h>

// nap::websocketclientendpoint run time class definition 
RTTI_BEGIN_CLASS(nap::WebSocketClientEndPoint)
	RTTI_PROPERTY("LogConnectionUpdates",	&nap::WebSocketClientEndPoint::mLogConnectionUpdates,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("LibraryLogLevel",		&nap::WebSocketClientEndPoint::mLibraryLogLevel,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
    template<typename T>
    class WebSocketClientEndPointImplementation : public WebSocketClientEndPointImplementationBase
    {
    public:
        WebSocketClientEndPointImplementation(nap::WebSocketClientEndPoint& endPoint) : WebSocketClientEndPointImplementationBase(endPoint), mClientEndPoint()
        {
        }

        bool init(utility::ErrorState &errorState) override
        {
            // Convert log levels
            mLogLevel = computeWebSocketLogLevel(mLibraryLogLevel);
            mAccessLogLevel = mLogConnectionUpdates ? websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload
                                                    : websocketpp::log::alevel::fail;

            // Initiate logging
            mClientEndPoint.clear_error_channels(websocketpp::log::elevel::all);
            mClientEndPoint.set_error_channels(mLogLevel);

            mClientEndPoint.clear_access_channels(websocketpp::log::alevel::all);
            mClientEndPoint.set_access_channels(mAccessLogLevel);

            // Init asio
            std::error_code stdec;
            mClientEndPoint.init_asio(stdec);
            if (stdec)
            {
                errorState.fail(stdec.message());
                return false;
            }

            return true;
        }

        void stop() override
        {
            // At this state we need to have an open end point and client thread
            assert(mRunning);
            assert(mClientTask.valid());

            // Make sure the end point doesn't restart
            mClientEndPoint.stop_perpetual();

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


        bool registerClient(nap::IWebSocketClient &client, utility::ErrorState &error)
        {
            // Get shared pointer to connection
            std::error_code stdec;
            wspp::ConnectionPtr client_connection = mClientEndPoint.get_connection(client.mURI, stdec);
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
            std::unique_ptr<WebSocketClientWrapper> meta_client(new WebSocketClientWrapper(client,
                                                                                           mClientEndPoint,
                                                                                           client_connection));
            mClients.emplace_back(std::move(meta_client));

            // Store connection handle in client
            client.mConnection = WebSocketConnection(client_connection->get_handle());

            // TRY to connect, this occurs on a background thread.
            // All connection related state changes are handled in the WebSocketClientWrapper.
            mClientEndPoint.connect(client_connection);

            return true;
        }

        void unregisterClient(const nap::IWebSocketClient &client) override
        {
            auto found_it = std::find_if(mClients.begin(), mClients.end(), [&](const auto& it)
            {
                return it->getResource() == &client;
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


        bool send(const WebSocketConnection& connection, const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error) override
        {
            std::error_code stdec;
            mClientEndPoint.send(connection.getConnectionHandle(), message, static_cast<wspp::OpCode>(code), stdec);
            if (stdec)
            {
                error.fail(stdec.message());
                return false;
            }
            return true;
        }


        bool send(const WebSocketConnection& connection, void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error) override
        {
            std::error_code stdec;
            mClientEndPoint.send(connection.getConnectionHandle(), payload, length, static_cast<wspp::OpCode>(code), stdec);
            if (stdec)
            {
                error.fail(stdec.message());
                return false;
            }
            return true;
        }

        bool start(nap::utility::ErrorState &error) override
        {
            // Ensure state
            assert(!mRunning);

            // Ensure connection exists when server disconnects
            mClientEndPoint.start_perpetual();

            // Run client in background
            mClientTask = std::async(std::launch::async, [this] { run(); });

            mRunning = true;

            return true;
        }
    private:
        void run()
        {
            mClientEndPoint.run();
        }

        T mClientEndPoint;
    };

    WebSocketClientEndPointImplementationBase::WebSocketClientEndPointImplementationBase(nap::WebSocketClientEndPoint &endPoint) : mEndPoint(endPoint)
    {
    }


	bool WebSocketClientEndPoint::init(utility::ErrorState& errorState)
	{
        mImplementation = std::make_unique<WebSocketClientEndPointImplementation<wspp::ClientEndPoint>>(*this);
        return mImplementation->init(errorState);
	}


	void WebSocketClientEndPoint::stop()
	{		
        mImplementation->stop();
	}


	bool WebSocketClientEndPoint::start(utility::ErrorState& error)
	{		
        mImplementation->start(error);
	}


	bool WebSocketClientEndPoint::send(const WebSocketConnection& connection, const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error)
	{
        return mImplementation->send(connection, message, code, error);
	}
	

	bool WebSocketClientEndPoint::send(const WebSocketConnection& connection, void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error)
	{
        return mImplementation->send(connection, payload, length, code, error);
	}


	bool WebSocketClientEndPoint::registerClient(IWebSocketClient& client, utility::ErrorState& error)
	{
        return mImplementation->registerClient(client, error);
	}


	void WebSocketClientEndPoint::unregisterClient(const IWebSocketClient& client)
	{
		mImplementation->unregisterClient(client);
	}

/*
    std::shared_ptr<asio::ssl::context> WebSocketClientEndPoint::onTlsInit(wspp::ConnectionHandle con)
    {
        /*
        namespace asio = websocketpp::lib::asio;
        // See https://wiki.mozilla.org/Security/Server_Side_TLS for more details about
        // the TLS modes. The code below demonstrates how to implement both the modern
        enum tls_mode {
            MOZILLA_INTERMEDIATE = 1,
            MOZILLA_MODERN = 2
        };

        auto tls_mode = MOZILLA_INTERMEDIATE;
        auto ctx = websocketpp::lib::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);

        try {
            if (tls_mode == MOZILLA_MODERN) {
                // Modern disables TLSv1
                ctx->set_options(asio::ssl::context::default_workarounds |
                                 asio::ssl::context::no_sslv2 |
                                 asio::ssl::context::no_sslv3 |
                                 asio::ssl::context::no_tlsv1 |
                                 asio::ssl::context::single_dh_use);
            } else {
                ctx->set_options(asio::ssl::context::default_workarounds |
                                 asio::ssl::context::no_sslv2 |
                                 asio::ssl::context::no_sslv3 |
                                 asio::ssl::context::single_dh_use);
            }

            ctx->use_certificate_chain_file(mCertificateFile);
            ctx->use_private_key_file(mPrivateKeyFile, asio::ssl::context::pem);

            std::string ciphers;
            if (tls_mode == MOZILLA_MODERN)
            {
                ciphers = "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!3DES:!MD5:!PSK";
            } else
            {
                ciphers = "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA:AES:CAMELLIA:DES-CBC3-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!aECDH:!EDH-DSS-DES-CBC3-SHA:!EDH-RSA-DES-CBC3-SHA:!KRB5-DES-CBC3-SHA";
            }

            if (SSL_CTX_set_cipher_list(ctx->native_handle() , ciphers.c_str()) != 1)
            {
                nap::Logger::warn("Error setting cipher list");
            }
        }
        catch (std::exception& e)
        {
            nap::Logger::error(*this, "Exception: %s", e.what());
        }

        return ctx;
    }
*/

	WebSocketClientWrapper::WebSocketClientWrapper(IWebSocketClient& client, wspp::ClientEndPoint& endPoint, wspp::ConnectionPtr connection) :
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


	WebSocketClientWrapper::~WebSocketClientWrapper()
	{
		// Not disconnected by server or client
		assert(!mOpen);
		mResource = nullptr;
		mEndPoint = nullptr;
	}


	void WebSocketClientWrapper::onConnectionOpened(wspp::ConnectionHandle connection)
	{
		assert(mResource != nullptr);
		mResource->connectionOpened();
		mOpen = true;
	}


	void WebSocketClientWrapper::onConnectionClosed(wspp::ConnectionHandle connection)
	{
		// Extract actual connection, must be valid at this point
		std::error_code stdec;
		wspp::ConnectionPtr cptr = mEndPoint->get_con_from_hdl(connection, stdec);
		assert(!stdec);

		assert(mResource != nullptr);
		mResource->connectionClosed(stdec.value(), stdec.message());
		mOpen = false;
	}


	void WebSocketClientWrapper::onConnectionFailed(wspp::ConnectionHandle connection)
	{
		// Extract actual connection, must be valid at this point
		std::error_code stdec;
		wspp::ConnectionPtr cptr = mEndPoint->get_con_from_hdl(connection, stdec);
		assert(!stdec);

		assert(mResource != nullptr);
		mResource->connectionFailed(stdec.value(), stdec.message());
		mOpen = false;
	}


	void WebSocketClientWrapper::onMessageReceived(wspp::ConnectionHandle connection, wspp::MessagePtr msg)
	{
		assert(mResource != nullptr);
		mResource->messageReceived(WebSocketMessage(msg));
	}


	bool WebSocketClientWrapper::disconnect(nap::utility::ErrorState& error)
	{
		if (mOpen)
		{
			// Get connection from handle, should be open by this point
			std::error_code stdec;
			wspp::ConnectionPtr cptr = mEndPoint->get_con_from_hdl(mHandle, stdec);
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

}