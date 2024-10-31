/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "websocketclientendpoint.h"

RTTI_DEFINE_CLASS(nap::WebSocketClientEndPoint, "Not secure (ws) websocket client endpoint, manages all client-server connections")

RTTI_BEGIN_CLASS(nap::SecureWebSocketClientEndPoint, "Secure (wss) websocket client endpoint, manages all client-server connections")
    RTTI_PROPERTY("CertificateChainFile", &nap::SecureWebSocketClientEndPoint::mCertificateChainFile, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::FileLink, "SSL certificate chain file")
    RTTI_PROPERTY("HostName", &nap::SecureWebSocketClientEndPoint::mHostName, nap::rtti::EPropertyMetaData::Default, "Hostname to verify against the certificate")
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

#ifdef WIN32
#define strcasecmp(s1, s2) _stricmp(s1, s2)
#endif

namespace nap
{
	/// Verify that one of the subject alternative names matches the given hostname
	static bool verifySubjectAlternativeName(const char* hostname, X509* cert)
	{
		STACK_OF(GENERAL_NAME)* san_names = NULL;

		san_names = (STACK_OF(GENERAL_NAME)*) X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL);
		if (san_names == NULL)
			return false;

		int san_names_count = sk_GENERAL_NAME_num(san_names);
		bool result = false;
		for (int i = 0; i < san_names_count; i++)
		{
			const GENERAL_NAME* current_name = sk_GENERAL_NAME_value(san_names, i);

			if (current_name->type != GEN_DNS)
				continue;

			// Make sure there isn't an embedded NUL character in the DNS name
			char const* dns_name = (char const*)ASN1_STRING_get0_data(current_name->d.dNSName);
			if (ASN1_STRING_length(current_name->d.dNSName) != strlen(dns_name))
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
	static bool verifyCommonName(char const* hostname, X509* cert)
	{
		// Find the position of the CN field in the Subject field of the certificate
		int common_name_loc = X509_NAME_get_index_by_NID(X509_get_subject_name(cert), NID_commonName, -1);
		if (common_name_loc < 0)
			return false;

		// Extract the CN field
		X509_NAME_ENTRY* common_name_entry = X509_NAME_get_entry(X509_get_subject_name(cert), common_name_loc);
		if (common_name_entry == NULL)
			return false;

		// Convert the CN field to a C string
		ASN1_STRING* common_name_asn1 = X509_NAME_ENTRY_get_data(common_name_entry);
		if (common_name_asn1 == NULL)
			return false;

		// Make sure there isn't an embedded NUL character in the CN
		char const* common_name_str = (char const*)ASN1_STRING_get0_data(common_name_asn1);
		if (ASN1_STRING_length(common_name_asn1) != strlen(common_name_str))
			return false;

		// Compare expected hostname with the CN
		return (strcasecmp(hostname, common_name_str) == 0);
	}


    bool SecureWebSocketClientEndPoint::start(nap::utility::ErrorState &error)
    {
        mEndPoint.set_tls_init_handler(std::bind(&SecureWebSocketClientEndPoint::onTLSInit, this, std::placeholders::_1));
        return WebSocketClientEndPointSetup<wspp::ConfigTLS>::start(error);
    }


    std::shared_ptr<asio::ssl::context> SecureWebSocketClientEndPoint::onTLSInit(websocketpp::connection_hdl hdl)
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
            ctx->set_verify_callback(bind(&SecureWebSocketClientEndPoint::verifyCertificate,
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
     * This code is derived from examples and documentation found at 
     * http://www.boost.org/doc/libs/1_61_0/doc/html/boost_asio/example/cpp03/ssl/client.cpp
     * and
     * https://github.com/iSECPartners/ssl-conservatory
     */
    bool SecureWebSocketClientEndPoint::verifyCertificate(const char * hostname, bool preverified, asio::ssl::verify_context& ctx)
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
            }
			else if(verifyCommonName(hostname, cert))
            {
                return true;
            }
			else
            {
                return false;
            }
        }
        return preverified;
    }
}
