// Local Includes
#include "websocketserver.h"

// External Includes
#include <nap/logger.h>

// nap::websocketserver run time class definition 
RTTI_BEGIN_CLASS(nap::WebsocketServer)
	RTTI_PROPERTY("Port", &nap::WebsocketServer::mPort, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	WebsocketServer::~WebsocketServer()			
	{
		stop();
	}


	bool WebsocketServer::init(utility::ErrorState& errorState)
	{
		return true;
	}


	bool WebsocketServer::start(utility::ErrorState& errorState)
	{
		// Create the end point
		mEndpoint = std::make_unique<ServerEndpoint>();

		// Initiate logging
		mEndpoint->set_error_channels(websocketpp::log::elevel::all);
		mEndpoint->set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);

		// Initialize asio
		mEndpoint->init_asio();

		mEndpoint->set_message_handler(std::bind(
			&WebsocketServer::messageHandler, this,
			std::placeholders::_1, std::placeholders::_2
		));

		// Listen to messages on specific port
		mEndpoint->listen(mPort);

		// Queues a connection accept operation
		mEndpoint->start_accept();

		// Start the Asio io_service run loop
		mEndpoint->run();

		return true;
	}


	void WebsocketServer::stop()
	{
		mEndpoint->stop_listening();
		mEndpoint.reset(nullptr);
	}


	void WebsocketServer::messageHandler(websocketpp::connection_hdl hdl, ServerEndpoint::message_ptr msg)
	{
		nap::Logger::info("Message Received!");
		nap::Logger::info(msg->get_payload());
		mEndpoint->send(hdl, msg->get_payload(), msg->get_opcode());
	}
}