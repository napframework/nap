// Local Includes
#include "websocketserver.h"
#include "websocketutils.h"

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
		uint32 log_level = computeWebSocketLogLevel(EWebSocketLogLevel::Error);
		mEndpoint = std::make_unique<WebSocketServerEndPoint>(mPort,  log_level, true);

		mEndpoint->setHandler(std::bind(
			&WebsocketServer::messageHandler, this,
			std::placeholders::_1, std::placeholders::_2
		));

		mEndpoint->open();

		return true;
	}


	void WebsocketServer::stop()
	{
		if (mEndpoint != nullptr)
		{
			mEndpoint->close();
			mEndpoint.reset(nullptr);
		}
	}


	void WebsocketServer::messageHandler(websocketpp::connection_hdl hdl, WebSocketServerEndPoint::PPServerEndPoint::message_ptr msg)
	{
		nap::Logger::info("Message Received!");
		nap::Logger::info(msg->get_payload());
		mEndpoint->send(msg->get_payload(), hdl, msg);
	}
}