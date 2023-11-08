/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "websocketserverendpoint.h"
#include "websocketserver.h"
#include "websocketticket.h"

// External Includes
#include <nap/logger.h>
#include <mathutils.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/error/en.h>

RTTI_BEGIN_ENUM(nap::WebSocketServerEndPoint::EAccessMode)
	RTTI_ENUM_VALUE(nap::WebSocketServerEndPoint::EAccessMode::EveryOne,	"Everyone"),
	RTTI_ENUM_VALUE(nap::WebSocketServerEndPoint::EAccessMode::Ticket,		"Ticket"),
	RTTI_ENUM_VALUE(nap::WebSocketServerEndPoint::EAccessMode::Reserved,	"Reserved")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::WebSocketServerEndPoint)
	RTTI_PROPERTY("AllowPortReuse",			&nap::WebSocketServerEndPoint::mAllowPortReuse,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("LogConnectionUpdates",	&nap::WebSocketServerEndPoint::mLogConnectionUpdates,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Port",					&nap::WebSocketServerEndPoint::mPort,						nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("IPAddress",				&nap::WebSocketServerEndPoint::mIPAddress,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AccessMode",				&nap::WebSocketServerEndPoint::mMode,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ConnectionLimit",		&nap::WebSocketServerEndPoint::mConnectionLimit,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("LibraryLogLevel",		&nap::WebSocketServerEndPoint::mLibraryLogLevel,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AllowControlOrigin",		&nap::WebSocketServerEndPoint::mAccessAllowControlOrigin,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clients",				&nap::WebSocketServerEndPoint::mClients,					nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::WebSocketServerEndPointTLS)
    RTTI_PROPERTY("CertificateFile",        &nap::WebSocketServerEndPointTLS::mCertificateFile,            nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::FileLink)
    RTTI_PROPERTY("PrivateKeyFile",         &nap::WebSocketServerEndPointTLS::mPrivateKeyFile,             nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::FileLink)
RTTI_END_CLASS


namespace nap
{
    WebSocketServerEndpointImplementation::WebSocketServerEndpointImplementation(nap::WebSocketServerEndPoint &endPoint) : mEndPoint(endPoint)
    {
    }

    template<typename T>
    class IWebSocketServerEndpoint : public WebSocketServerEndpointImplementation
    {
    public:
        IWebSocketServerEndpoint(WebSocketServerEndPoint& endPoint) : WebSocketServerEndpointImplementation(endPoint)
        {
            mMode = mEndPoint.mMode;
            mConnectionLimit = mEndPoint.mConnectionLimit;
            mPort = mEndPoint.mPort;
            mLogConnectionUpdates = mEndPoint.mLogConnectionUpdates;
            mAllowPortReuse = mEndPoint.mAllowPortReuse;
            mLibraryLogLevel = mEndPoint.mLibraryLogLevel;
            mClients = mEndPoint.mClients;
            mAccessAllowControlOrigin = mEndPoint.mAccessAllowControlOrigin;
            mIPAddress = mEndPoint.mIPAddress;
        }

        virtual ~IWebSocketServerEndpoint() = default;

        virtual bool start(nap::utility::ErrorState& error) override
        {
            // Listen to messages on this specific port and ip address (if given)
            std::error_code stdec;
            if (mIPAddress.empty())
            {
                mServerEndPoint.listen(static_cast<uint16>(mPort), stdec);
            }
            else
            {
                mServerEndPoint.listen(mIPAddress, utility::stringFormat("%d", static_cast<uint16>(mPort)), stdec);
            }

            // Contains the error when opening port fails.
            if (stdec)
            {
                error.fail(stdec.message());
                return false;
            }

            attachTLSHandler();

            // Queues a connection accept operation
            mServerEndPoint.start_accept(stdec);
            if (stdec)
            {
                error.fail(stdec.message());
                return false;
            }

            // Run until stopped
            mServerTask  = std::async(std::launch::async, [this] { run(); });
            mRunning = true;

            return true;
        }


        virtual bool send(const WebSocketConnection& connection, const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error) override
        {
            std::error_code stdec;
            mServerEndPoint.send(connection.getConnectionHandle(), message, static_cast<wspp::OpCode>(code), stdec);
            if (stdec)
            {
                error.fail(stdec.message());
                return false;
            }
            return true;
        }

        virtual bool send(const WebSocketConnection& connection, void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error) override
        {
            std::error_code stdec;
            mServerEndPoint.send(connection.getConnectionHandle(), payload, length, static_cast<wspp::OpCode>(code), stdec);
            if (stdec)
            {
                error.fail(stdec.message());
                return false;
            }
            return true;
        }

        virtual bool broadcast(const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error) override
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

        virtual bool broadcast(void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error) override
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

        virtual std::string getHostName(const WebSocketConnection& connection) override
        {
            std::error_code stdec;
            auto cptr = mServerEndPoint.get_con_from_hdl(connection.getConnectionHandle(), stdec);
            return stdec ? "" : cptr->get_host();
        }

        virtual void getHostNames(std::vector<std::string>& outHosts) override
        {
            outHosts.clear();
            std::error_code stdec;
            std::lock_guard<std::mutex> lock(mConnectionMutex);
            outHosts.reserve(mConnections.size());
            for (auto& connection : mConnections)
            {
                auto cptr = mServerEndPoint.get_con_from_hdl(connection, stdec);
                if (!stdec)
                    outHosts.emplace_back(cptr->get_host());
            }
        }

        virtual int getConnectionCount() override
        {
            std::lock_guard<std::mutex> lock(mConnectionMutex);
            return mConnections.size();
        }

        virtual bool acceptsNewConnections() override
        {
            if (mConnectionLimit < 0)
                return true;

            std::lock_guard<std::mutex> lock(mConnectionMutex);
            return mConnections.size() < mConnectionLimit;
        }

        virtual void stop() override
        {
            if (mRunning)
            {
                // Stop listening for new connections
                std::error_code stdec;
                mServerEndPoint.stop_listening(stdec);
                if (stdec)
                {
                    assert(false);
                    nap::Logger::error("%s: %s", mEndPoint.mID.c_str(), stdec.message().c_str());
                }

                // Close all client connections
                utility::ErrorState napec;
                if (!disconnect(napec))
                {
                    assert(false);
                    nap::Logger::error("%s: %s", mEndPoint.mID.c_str(), napec.toString().c_str());
                }

                // Explicitly stop
                mEndPoint.stop();

                // Wait for thread to finish
                assert(mServerTask.valid());
                mServerTask.wait();
                mRunning = false;
            }
        }

        bool init(utility::ErrorState &errorState) override
        {
            // Convert log levels
            mLogLevel = computeWebSocketLogLevel(mLibraryLogLevel);
            mAccessLogLevel = mLogConnectionUpdates ? websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload
                                                    : websocketpp::log::alevel::fail;

            // Initiate logging
            mServerEndPoint.clear_error_channels(websocketpp::log::elevel::all);
            mServerEndPoint.set_error_channels(mLogLevel);

            mServerEndPoint.clear_access_channels(websocketpp::log::alevel::all);
            mServerEndPoint.set_access_channels(mAccessLogLevel);

            // If the endpoint can be re-used by other processes
            mServerEndPoint.set_reuse_addr(mAllowPortReuse);

            // Init asio
            std::error_code stdec;
            mServerEndPoint.init_asio(stdec);
            if (stdec)
            {
                errorState.fail(stdec.message());
                return false;
            }

            // Install connection open / closed handlers
            mServerEndPoint.set_http_handler([this](auto && PH1) { onHTTP(std::forward<decltype(PH1)>(PH1)); });
            mServerEndPoint.set_open_handler([this](auto && PH1) { onConnectionOpened(std::forward<decltype(PH1)>(PH1)); });
            mServerEndPoint.set_close_handler([this](auto && PH1) { onConnectionClosed(std::forward<decltype(PH1)>(PH1)); });
            mServerEndPoint.set_fail_handler([this](auto && PH1) { onConnectionFailed(std::forward<decltype(PH1)>(PH1)); });
            mServerEndPoint.set_validate_handler([this](auto && PH1) { return onValidate(std::forward<decltype(PH1)>(PH1)); });
            mServerEndPoint.set_ping_handler([this](auto && PH1, auto && PH2) { return onPing(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });

            // Install message handler
            mServerEndPoint.set_message_handler([this](auto && PH1, auto && PH2) { onMessageReceived(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });

            // Create unique hashes out of tickets
            // Server side tickets are required to have a password and username
            for (const auto& ticket : mClients)
                mClientHashes.emplace(ticket->toHash());
            return true;
        }

        void onConnectionClosed(const nap::WebSocketConnection &connection) override
        {
            std::error_code stdec;
            auto cptr = mServerEndPoint.get_con_from_hdl(connection.getConnectionHandle(), stdec);
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
                    auto client_ptr = mServerEndPoint.get_con_from_hdl(it, stdec);
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

        void registerListener(IWebSocketServer& server) override
        {
            std::unique_lock<std::mutex> lock(mListenerMutex);
            mListeners.push_back(&server);
        }

        void unregisterListener(IWebSocketServer& server) override
        {
            std::unique_lock<std::mutex> lock(mListenerMutex);
            mListeners.erase(std::remove(mListeners.begin(), mListeners.end(), &server), mListeners.end());
        }

        void onConnectionFailed(const nap::WebSocketConnection &connection) override
        {
            std::error_code stdec;
            auto cptr = mServerEndPoint.get_con_from_hdl(connection.getConnectionHandle(), stdec);
            if (stdec)
            {
                nap::Logger::error(stdec.message());
                return;
            }

            for (IWebSocketServer* listener : mListeners)
                listener->onConnectionFailed(WebSocketConnection(connection), cptr->get_ec().value(), cptr->get_ec().message());
        }

        void onConnectionOpened(const nap::WebSocketConnection &connection) override
        {
            std::error_code stdec;
            auto cptr = mServerEndPoint.get_con_from_hdl(connection.getConnectionHandle(), stdec);
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

        void onHTTP(const nap::WebSocketConnection &connection) override
        {
            // Get handle to connection
            std::error_code stdec;
            auto conp = mServerEndPoint.get_con_from_hdl(connection.getConnectionHandle(), stdec);
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
            if (mMode == WebSocketServerEndPoint::EAccessMode::EveryOne)
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
            if(!ticket.init(error))
            {
                conp->append_header("WWW-Authenticate", "NAPUserPass");
                conp->set_status(websocketpp::http::status_code::unauthorized,
                                 utility::stringFormat("invalid username or password: %s", error.toString().c_str()));
                return;
            }

            // If we only allow specific clients we extract the password here.
            // The pass is used in combination with the username to create a validation hash.
            if (mMode == WebSocketServerEndPoint::EAccessMode::Reserved)
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

        void onMessageReceived(const nap::WebSocketConnection &connection, const nap::WebSocketMessage &msg) override
        {
            WebSocketMessage message(msg);
            for (IWebSocketServer* listener : mListeners)
                listener->onMessageReceived(WebSocketConnection(connection), message);
        }

        bool onPing(const nap::WebSocketConnection &connection, const std::string &message) override
        {
            return true;
        }

        bool onValidate(const nap::WebSocketConnection &connection) override
        {
            // Get connection handle
            std::error_code stdec;
            auto conp = mServerEndPoint.get_con_from_hdl(connection.getConnectionHandle(), stdec);
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
            if (mMode == WebSocketServerEndPoint::EAccessMode::EveryOne)
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
            if (mMode == WebSocketServerEndPoint::EAccessMode::Ticket)
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

        std::shared_ptr<asio::ssl::context> onTlsInit(wspp::ConnectionHandle con)
        {
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
                nap::Logger::error("Exception: %s", e.what());
            }
            return ctx;
        }

        void run() override
        {
            mServerEndPoint.run();
        }

        bool disconnect(utility::ErrorState& error) override
        {
            std::lock_guard<std::mutex> lock(mConnectionMutex);
            bool success = true;
            for (auto& connection : mConnections)
            {
                std::error_code stdec;
                mServerEndPoint.close(connection, websocketpp::close::status::going_away, "disconnected", stdec);
                if (stdec)
                {
                    error.fail(stdec.message());
                    success = false;
                }
            }
            mConnections.clear();
            return success;
        }

        void attachTLSHandler() override{}
    private:
        T mServerEndPoint;
    };

    template<>
    IWebSocketServerEndpoint<wspp::ServerEndPointTLS>::IWebSocketServerEndpoint(WebSocketServerEndPoint& endPoint) : WebSocketServerEndpointImplementation(endPoint)
    {
        mMode = mEndPoint.mMode;
        mConnectionLimit = mEndPoint.mConnectionLimit;
        mPort = mEndPoint.mPort;
        mLogConnectionUpdates = mEndPoint.mLogConnectionUpdates;
        mAllowPortReuse = mEndPoint.mAllowPortReuse;
        mLibraryLogLevel = mEndPoint.mLibraryLogLevel;
        mClients = mEndPoint.mClients;
        mAccessAllowControlOrigin = mEndPoint.mAccessAllowControlOrigin;
        mIPAddress = mEndPoint.mIPAddress;

        assert(endPoint.get_type().is_derived_from<WebSocketServerEndPointTLS>());
        auto& tlsEndPoint = static_cast<WebSocketServerEndPointTLS&>(endPoint);
        mCertificateFile = tlsEndPoint.mCertificateFile;
        mPrivateKeyFile = tlsEndPoint.mPrivateKeyFile;
    }

    template<>
    void IWebSocketServerEndpoint<wspp::ServerEndPointTLS>::attachTLSHandler()
    {
        mServerEndPoint.set_tls_init_handler([this](auto && PH1) { return onTlsInit(std::forward<decltype(PH1)>(PH1)); });
    }

	WebSocketServerEndPoint::WebSocketServerEndPoint()
	{
	}


	bool WebSocketServerEndPoint::start(nap::utility::ErrorState& error)
	{
        return mImplementation->start(error);
	}


	bool WebSocketServerEndPoint::isOpen() const
	{
		return mImplementation->isOpen();
	}


	bool WebSocketServerEndPoint::send(const WebSocketConnection& connection, const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error)
	{
        return mImplementation->send(connection, message, code, error);
	}


	bool WebSocketServerEndPoint::send(const WebSocketConnection& connection, void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error)
	{
        return mImplementation->send(connection, payload, length, code, error);
	}


	bool WebSocketServerEndPoint::broadcast(const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error)
	{
        return mImplementation->broadcast(message, code, error);
	}


	bool WebSocketServerEndPoint::broadcast(void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error)
	{
        return mImplementation->broadcast(payload, length, code, error);
	}


	std::string WebSocketServerEndPoint::getHostName(const WebSocketConnection& connection)
	{
        return mImplementation->getHostName(connection);
	}


	void WebSocketServerEndPoint::getHostNames(std::vector<std::string>& outHosts)
	{
        mImplementation->getHostNames(outHosts);
	}


	int WebSocketServerEndPoint::getConnectionCount()
	{
        return mImplementation->getConnectionCount();
	}


	bool WebSocketServerEndPoint::acceptsNewConnections()
	{
        return mImplementation->acceptsNewConnections();
	}


	void WebSocketServerEndPoint::stop()
	{
        mImplementation->stop();
	}


	void WebSocketServerEndPoint::registerListener(IWebSocketServer& server)
	{
        mImplementation->registerListener(server);
	}


	void WebSocketServerEndPoint::unregisterListener(IWebSocketServer& server)
	{
        mImplementation->unregisterListener(server);
	}


	WebSocketServerEndPoint::~WebSocketServerEndPoint()
	{
        if(mImplementation)
            mImplementation->stop();
	}


	bool WebSocketServerEndPoint::init(utility::ErrorState& errorState)
	{
        mImplementation = std::make_unique<IWebSocketServerEndpoint<wspp::ServerEndPoint>>(*this);
        return mImplementation->init(errorState);
	}


    bool WebSocketServerEndPointTLS::init(utility::ErrorState &errorState)
    {
        mImplementation = std::make_unique<IWebSocketServerEndpoint<wspp::ServerEndPointTLS>>(*this);
        return mImplementation->init(errorState);
    }
}
