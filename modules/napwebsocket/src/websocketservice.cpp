// Local Includes
#include "websocketservice.h"

// External Includes
#include <nap/logger.h>
#include <websocketpp/server.hpp>
#include <utility/stringutils.h>
#include <websocketpp/config/asio_no_tls.hpp>


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WebsocketService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	WebsocketService::WebsocketService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


    bool WebsocketService::init(nap::utility::ErrorState& errorState)
    {
		using server = websocketpp::server<websocketpp::config::asio>;

		server m_endpoint;

		// Set logging settings
		m_endpoint.set_error_channels(websocketpp::log::elevel::all);
		m_endpoint.set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);

		// Initialize Asio
		m_endpoint.init_asio();

		// Set the default message handler to the echo handler
		/*
		m_endpoint.set_message_handler(std::bind(
			&utility_server::echo_handler, this,
			std::placeholders::_1, std::placeholders::_2
		));
		*/

		return true;
    }    
}
