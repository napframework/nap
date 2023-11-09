/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "websocketserverendpoint.h"
#include "websocketserver.h"
#include "websocketticket.h"
#include "utility/fileutils.h"

// External Includes
#include <nap/logger.h>
#include <mathutils.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/error/en.h>

RTTI_BEGIN_ENUM(nap::WebSocketServerEndPointBase::EAccessMode)
        RTTI_ENUM_VALUE(nap::WebSocketServerEndPointBase::EAccessMode::EveryOne, "Everyone"),
        RTTI_ENUM_VALUE(nap::WebSocketServerEndPointBase::EAccessMode::Ticket, "Ticket"),
        RTTI_ENUM_VALUE(nap::WebSocketServerEndPointBase::EAccessMode::Reserved, "Reserved")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WebSocketServerEndPointBase)
        RTTI_PROPERTY("AllowPortReuse", &nap::WebSocketServerEndPointBase::mAllowPortReuse, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("LogConnectionUpdates", &nap::WebSocketServerEndPointBase::mLogConnectionUpdates, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("Port", &nap::WebSocketServerEndPointBase::mPort, nap::rtti::EPropertyMetaData::Required)
        RTTI_PROPERTY("IPAddress", &nap::WebSocketServerEndPointBase::mIPAddress, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("AccessMode", &nap::WebSocketServerEndPointBase::mMode, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("ConnectionLimit", &nap::WebSocketServerEndPointBase::mConnectionLimit, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("LibraryLogLevel", &nap::WebSocketServerEndPointBase::mLibraryLogLevel, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("AllowControlOrigin", &nap::WebSocketServerEndPointBase::mAccessAllowControlOrigin, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("Clients", &nap::WebSocketServerEndPointBase::mClients, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::WebSocketServerEndPointNoTLS)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::WebSocketServerEndPointTLS)
    RTTI_PROPERTY("Certificate", &nap::WebSocketServerEndPointTLS::mCertificateFile, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::FileLink)
    RTTI_PROPERTY("PrivateKey", &nap::WebSocketServerEndPointTLS::mPrivateKeyFile, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::FileLink)
    RTTI_PROPERTY("Passphrase", &nap::WebSocketServerEndPointTLS::mPassphrase, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
    template<typename config>
    WebSocketServerEndPoint<config>::WebSocketServerEndPoint()
    {
    }

    template<typename config>
    bool WebSocketServerEndPoint<config>::start(nap::utility::ErrorState& error)
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
        mServerTask  = std::async(std::launch::async, std::bind(&WebSocketServerEndPoint<config>::run, this));
        mRunning = true;

        return true;
    }

    template<typename config>
    bool WebSocketServerEndPoint<config>::isOpen() const
    {
        return mRunning;
    }

    template<typename config>
    bool WebSocketServerEndPoint<config>::send(const WebSocketConnection& connection, const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error)
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
    bool WebSocketServerEndPoint<config>::send(const WebSocketConnection& connection, void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error)
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
    bool WebSocketServerEndPoint<config>::broadcast(const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error)
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
    bool WebSocketServerEndPoint<config>::broadcast(void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error)
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
    std::string WebSocketServerEndPoint<config>::getHostName(const WebSocketConnection& connection)
    {
        std::error_code stdec;
        auto cptr = mEndPoint.get_con_from_hdl(connection.mConnection, stdec);
        return stdec ? "" : cptr->get_host();
    }

    template<typename config>
    void WebSocketServerEndPoint<config>::getHostNames(std::vector<std::string>& outHosts)
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
    int WebSocketServerEndPoint<config>::getConnectionCount()
    {
        std::lock_guard<std::mutex> lock(mConnectionMutex);
        return mConnections.size();
    }

    template<typename config>
    bool WebSocketServerEndPoint<config>::acceptsNewConnections()
    {
        if (mConnectionLimit < 0)
            return true;

        std::lock_guard<std::mutex> lock(mConnectionMutex);
        return mConnections.size() < mConnectionLimit;
    }


    template<typename config>
    void WebSocketServerEndPoint<config>::run()
    {
        // Start running until stopped
        mEndPoint.run();
    }


    template<typename config>
    void WebSocketServerEndPoint<config>::stop()
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
    void WebSocketServerEndPoint<config>::registerListener(IWebSocketServer& server)
    {
        std::unique_lock<std::mutex> lock(mListenerMutex);
        mListeners.push_back(&server);
    }


    template<typename config>
    void WebSocketServerEndPoint<config>::unregisterListener(IWebSocketServer& server)
    {
        std::unique_lock<std::mutex> lock(mListenerMutex);
        mListeners.erase(std::remove(mListeners.begin(), mListeners.end(), &server), mListeners.end());
    }


    template<typename config>
    WebSocketServerEndPoint<config>::~WebSocketServerEndPoint()
    {
        stop();
    }


    template<typename config>
    bool WebSocketServerEndPoint<config>::init(utility::ErrorState& errorState)
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
        mEndPoint.set_http_handler(std::bind(&WebSocketServerEndPoint<config>::onHTTP, this, std::placeholders::_1));
        mEndPoint.set_open_handler(std::bind(&WebSocketServerEndPoint<config>::onConnectionOpened, this, std::placeholders::_1));
        mEndPoint.set_close_handler(std::bind(&WebSocketServerEndPoint<config>::onConnectionClosed, this, std::placeholders::_1));
        mEndPoint.set_fail_handler(std::bind(&WebSocketServerEndPoint<config>::onConnectionFailed, this, std::placeholders::_1));
        mEndPoint.set_validate_handler(std::bind(&WebSocketServerEndPoint<config>::onValidate, this, std::placeholders::_1));
        mEndPoint.set_ping_handler(std::bind(&WebSocketServerEndPoint<config>::onPing, this, std::placeholders::_1, std::placeholders::_2));

        // Install message handler
        mEndPoint.set_message_handler(std::bind(
                &WebSocketServerEndPoint<config>::onMessageReceived, this,
                std::placeholders::_1, std::placeholders::_2
        ));

        // Create unique hashes out of tickets
        // Server side tickets are required to have a password and username
        for (const auto& ticket : mClients)
            mClientHashes.emplace(ticket->toHash());
        return true;
    }


    template<typename config>
    void WebSocketServerEndPoint<config>::onConnectionOpened(wspp::ConnectionHandle connection)
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
    void WebSocketServerEndPoint<config>::onConnectionClosed(wspp::ConnectionHandle connection)
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
    void WebSocketServerEndPoint<config>::onConnectionFailed(wspp::ConnectionHandle connection)
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
    void WebSocketServerEndPoint<config>::onMessageReceived(wspp::ConnectionHandle con, wspp::MessagePtr msg)
    {
        WebSocketMessage message(msg);
        for (IWebSocketServer* listener : mListeners)
            listener->onMessageReceived(WebSocketConnection(con), message);
    }


    template<typename config>
    void WebSocketServerEndPoint<config>::onHTTP(wspp::ConnectionHandle con)
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
        if(!ticket.init(error))
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
    bool WebSocketServerEndPoint<config>::onValidate(wspp::ConnectionHandle con)
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
    bool WebSocketServerEndPoint<config>::onPing(wspp::ConnectionHandle con, std::string msg)
    {
        return true;
    }


    template<typename config>
    bool WebSocketServerEndPoint<config>::disconnect(nap::utility::ErrorState& error)
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


    bool WebSocketServerEndPointTLS::init(utility::ErrorState &errorState)
    {
        if(!errorState.check(utility::fileExists(mCertificateFile), "Certificate file %s not found", mCertificateFile.c_str()))
            return false;

        if(!errorState.check(utility::fileExists(mPrivateKeyFile), "PrivateKey file %s not found", mPrivateKeyFile.c_str()))
            return false;

        return WebSocketServerEndPoint<wspp::ConfigTLS>::init(errorState);
    }


    bool WebSocketServerEndPointTLS::start(nap::utility::ErrorState &error)
    {
        mEndPoint.set_tls_init_handler(std::bind(&WebSocketServerEndPointTLS::onTLSInit, this, std::placeholders::_1));
        return WebSocketServerEndPoint<wspp::ConfigTLS>::start(error);
    }


    std::shared_ptr<asio::ssl::context> WebSocketServerEndPointTLS::onTLSInit(wspp::ConnectionHandle con)
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
                                 asio::ssl::context::no_tlsv1);
            } else {
                ctx->set_options(asio::ssl::context::default_workarounds |
                                 asio::ssl::context::no_sslv2 |
                                 asio::ssl::context::no_sslv3);
            }

            ctx->set_password_callback([this](std::size_t, asio::ssl::context_base::password_purpose) { return mPassphrase; });
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
}
