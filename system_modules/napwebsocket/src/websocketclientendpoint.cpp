/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "websocketclientendpoint.h"
#include "websocketclient.h"
#include "websocketmessage.h"

// External Includes
#include <nap/logger.h>

// nap::websocketclientendpoint run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WebSocketClientEndPointBase)
	RTTI_PROPERTY("LogConnectionUpdates", &nap::WebSocketClientEndPointBase::mLogConnectionUpdates, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("LibraryLogLevel", &nap::WebSocketClientEndPointBase::mLibraryLogLevel, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::WebSocketClientEndPointNoTLS)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::WebSocketClientEndPointTLS)
    RTTI_PROPERTY("CertificateChainFile", &nap::WebSocketClientEndPointTLS::mCertificateChainFile, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::FileLink)
    RTTI_PROPERTY("HostName", &nap::WebSocketClientEndPointTLS::mHostName, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
    bool verifySubjectAlternativeName(const char * hostname, X509 * cert);
    bool verifyCommonName(const char * hostname, X509 * cert);

    template<typename config>
	bool WebSocketClientEndPoint<config>::init(utility::ErrorState& errorState)
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
    void WebSocketClientEndPoint<config>::stop()
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
    bool WebSocketClientEndPoint<config>::start(utility::ErrorState& error)
	{		
		// Ensure state
		assert(!mRunning);

		// Ensure connection exists when server disconnects
		mEndPoint.start_perpetual();

		// Run client in background
		mClientTask = std::async(std::launch::async, std::bind(&WebSocketClientEndPoint<config>::run, this));

		mRunning = true;

		return true;
	}

    template<typename config>
    bool WebSocketClientEndPoint<config>::send(const WebSocketConnection& connection, const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error)
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
    bool WebSocketClientEndPoint<config>::send(const WebSocketConnection& connection, void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error)
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
    void WebSocketClientEndPoint<config>::run()
	{
		// Start running until stopped
		mEndPoint.run();
	}

    template<typename config>
    bool WebSocketClientEndPoint<config>::registerClient(IWebSocketClient& client, utility::ErrorState& error)
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
    void WebSocketClientEndPoint<config>::unregisterClient(const IWebSocketClient& client)
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


    bool WebSocketClientEndPointTLS::start(nap::utility::ErrorState &error)
    {
        mEndPoint.set_tls_init_handler(std::bind(&WebSocketClientEndPointTLS::onTLSInit, this, std::placeholders::_1));
        return WebSocketClientEndPoint<wspp::ConfigTLS>::start(error);
    }


    std::shared_ptr<asio::ssl::context> WebSocketClientEndPointTLS::onTLSInit(websocketpp::connection_hdl hdl)
    {
        auto ctx = websocketpp::lib::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);
        auto connection = mEndPoint.get_con_from_hdl(hdl);

        try
        {
            ctx->set_options(asio::ssl::context::default_workarounds |
                             asio::ssl::context::no_sslv2 |
                             asio::ssl::context::no_sslv3 |
                             asio::ssl::context::single_dh_use);

            ctx->set_verify_mode(asio::ssl::verify_peer);
            ctx->set_verify_callback(bind(&WebSocketClientEndPointTLS::verifyCertificate,
                                          this,
                                          mHostName.c_str(),
                                          std::placeholders::_1, std::placeholders::_2));

            // Here we load the CA certificates of all CA's that this client trusts.
            ctx->load_verify_file(mCertificateChainFile);
        }
        catch (std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
        return ctx;
    }

    /**
     * This code is derived from examples and documentation found ato00po
     * http://www.boost.org/doc/libs/1_61_0/doc/html/boost_asio/example/cpp03/ssl/client.cpp
     * and
     * https://github.com/iSECPartners/ssl-conservatory
     */
    bool WebSocketClientEndPointTLS::verifyCertificate(const char * hostname, bool preverified, asio::ssl::verify_context& ctx)
    {
        // The verify callback can be used to check whether the certificate that is
        // being presented is valid for the peer. For example, RFC 2818 describes
        // the steps involved in doing this for HTTPS. Consult the OpenSSL
        // documentation for more details. Note that the callback is called once
        // for each certificate in the certificate chain, starting from the root
        // certificate authority.

        // Retrieve the depth of the current cert in the chain. 0 indicates the
        // actual server cert, upon which we will perform extra validation
        // (specifically, ensuring that the hostname matches. For other certs we
        // will use the 'preverified' flag from Asio, which incorporates a number of
        // non-implementation specific OpenSSL checking, such as the formatting of
        // certs and the trusted status based on the CA certs we imported earlier.
        int depth = X509_STORE_CTX_get_error_depth(ctx.native_handle());

        // if we are on the final cert and everything else checks out, ensure that
        // the hostname is present on the list of SANs or the common name (CN).
        if(depth == 0 && preverified)
        {
            X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());

            if(verifySubjectAlternativeName(hostname, cert))
            {
                return true;
            }else if(verifyCommonName(hostname, cert))
            {
                return true;
            }else
            {
                return false;
            }
        }

        return preverified;
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
	WebSocketClientWrapper<config>::~WebSocketClientWrapper()
	{
		// Not disconnected by server or client
		assert(!mOpen);
		mResource = nullptr;
		mEndPoint = nullptr;
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


    /// Verify that one of the subject alternative names matches the given hostname
    bool verifySubjectAlternativeName(const char * hostname, X509 * cert)
    {
        STACK_OF(GENERAL_NAME) * san_names = NULL;

        san_names = (STACK_OF(GENERAL_NAME) *) X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL);
        if(san_names == NULL)
        {
            return false;
        }

        int san_names_count = sk_GENERAL_NAME_num(san_names);

        bool result = false;

        for(int i = 0; i < san_names_count; i++)
        {
            const GENERAL_NAME * current_name = sk_GENERAL_NAME_value(san_names, i);

            if (current_name->type != GEN_DNS)
            {
                continue;
            }

            char const * dns_name = (char const *) ASN1_STRING_get0_data(current_name->d.dNSName);

            // Make sure there isn't an embedded NUL character in the DNS name
            if(ASN1_STRING_length(current_name->d.dNSName) != strlen(dns_name))
            {
                break;
            }
            // Compare expected hostname with the CN
            result = (strcasecmp(hostname, dns_name) == 0);
        }
        sk_GENERAL_NAME_pop_free(san_names, GENERAL_NAME_free);

        return result;
    }

    /// Verify that the certificate common name matches the given hostname
    bool verifyCommonName(char const * hostname, X509 * cert)
    {
        // Find the position of the CN field in the Subject field of the certificate
        int common_name_loc = X509_NAME_get_index_by_NID(X509_get_subject_name(cert), NID_commonName, -1);
        if(common_name_loc < 0)
        {
            return false;
        }

        // Extract the CN field
        X509_NAME_ENTRY * common_name_entry = X509_NAME_get_entry(X509_get_subject_name(cert), common_name_loc);
        if(common_name_entry == NULL)
        {
            return false;
        }

        // Convert the CN field to a C string
        ASN1_STRING * common_name_asn1 = X509_NAME_ENTRY_get_data(common_name_entry);
        if(common_name_asn1 == NULL)
        {
            return false;
        }

        char const * common_name_str = (char const *) ASN1_STRING_get0_data(common_name_asn1);

        // Make sure there isn't an embedded NUL character in the CN
        if(ASN1_STRING_length(common_name_asn1) != strlen(common_name_str))
        {
            return false;
        }

        // Compare expected hostname with the CN
        return (strcasecmp(hostname, common_name_str) == 0);
    }
}