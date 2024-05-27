/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "websocketserverendpoint.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/logger.h>

RTTI_BEGIN_ENUM(nap::SecureWebSocketServerEndPoint::ETLSMode)
	RTTI_ENUM_VALUE(nap::SecureWebSocketServerEndPoint::ETLSMode::Intermediate,		"Intermediate"),
	RTTI_ENUM_VALUE(nap::SecureWebSocketServerEndPoint::ETLSMode::Modern,			"Modern")
RTTI_END_ENUM

RTTI_DEFINE_CLASS(nap::WebSocketServerEndPoint, "Not secured websocket (ws) server end point connection")

RTTI_BEGIN_CLASS(nap::SecureWebSocketServerEndPoint, "Secured websocket (wss) server end point connection, adds transport layer security (TLS) using the provided certificate and key")
	RTTI_PROPERTY("Mode",			&nap::SecureWebSocketServerEndPoint::mMode,				nap::rtti::EPropertyMetaData::Default, "TLS configuration mode")
    RTTI_PROPERTY("Certificate",	&nap::SecureWebSocketServerEndPoint::mCertificateFile,	nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::FileLink, "TLS certificate file (.pem)")
    RTTI_PROPERTY("PrivateKey",		&nap::SecureWebSocketServerEndPoint::mPrivateKeyFile,	nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::FileLink, "TLS private key file (.key)")
    RTTI_PROPERTY("Passphrase",		&nap::SecureWebSocketServerEndPoint::mPassphrase,		nap::rtti::EPropertyMetaData::Default, "Private key password, only required when key is generated with one")
RTTI_END_CLASS

namespace nap
{
    bool SecureWebSocketServerEndPoint::init(utility::ErrorState &errorState)
    {
        if(!errorState.check(utility::fileExists(mCertificateFile), "Certificate file %s not found", mCertificateFile.c_str()))
            return false;

        if(!errorState.check(utility::fileExists(mPrivateKeyFile), "PrivateKey file %s not found", mPrivateKeyFile.c_str()))
            return false;

        return WebSocketServerEndPointSetup<wspp::ConfigTLS>::init(errorState);
    }


    bool SecureWebSocketServerEndPoint::start(nap::utility::ErrorState &error)
    {
        mEndPoint.set_tls_init_handler(std::bind(&SecureWebSocketServerEndPoint::onTLSInit, this, std::placeholders::_1));
        return WebSocketServerEndPointSetup<wspp::ConfigTLS>::start(error);
    }


    std::shared_ptr<asio::ssl::context> SecureWebSocketServerEndPoint::onTLSInit(wspp::ConnectionHandle con)
    {
        namespace asio = websocketpp::lib::asio;
        auto ctx = websocketpp::lib::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);

        try
        {
			switch (mMode)
			{
			case ETLSMode::Modern:
				{
					// Modern disables TLSv1
					ctx->set_options(asio::ssl::context::default_workarounds |
						asio::ssl::context::no_sslv2 |
						asio::ssl::context::no_sslv3 |
						asio::ssl::context::no_tlsv1);
					break;
				}
			case ETLSMode::Intermediate:
				{
					ctx->set_options(asio::ssl::context::default_workarounds |
						asio::ssl::context::no_sslv2 |
						asio::ssl::context::no_sslv3);
					break;
				}
			default:
				assert(false);
				break;
			}

            ctx->set_password_callback([this](std::size_t, asio::ssl::context_base::password_purpose) { return mPassphrase; });
            ctx->use_certificate_chain_file(mCertificateFile);
            ctx->use_private_key_file(mPrivateKeyFile, asio::ssl::context::pem);

            std::string ciphers;
			switch (mMode)
			{
				case ETLSMode::Modern:
				{
					ciphers = "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!3DES:!MD5:!PSK";
					break;
				}
				case ETLSMode::Intermediate:
				{
					ciphers = "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA:AES:CAMELLIA:DES-CBC3-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!aECDH:!EDH-DSS-DES-CBC3-SHA:!EDH-RSA-DES-CBC3-SHA:!KRB5-DES-CBC3-SHA";
					break;
				}
				default:
				{
					assert(false);
					break;
				}
			}

            if(SSL_CTX_set_cipher_list(ctx->native_handle() , ciphers.c_str()) != 1)
            {
                nap::Logger::error("Error setting cipher list");
            }
        }
        catch (std::exception& e)
        {
            nap::Logger::error("Exception: %s", e.what());
        }
        return ctx;
    }
}
