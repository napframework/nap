// Local Includes
#include "websocketserver.h"
#include "websocketutils.h"

// External Includes
#include <nap/logger.h>

// nap::websocketserver run time class definition 
RTTI_BEGIN_CLASS(nap::WebsocketServer)
	RTTI_PROPERTY("Port",					&nap::WebsocketServer::mPort,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("LogConnectionUpdates",	&nap::WebsocketServer::mLogConnectionUpdates,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("LibraryLogLevel",		&nap::WebsocketServer::mLibraryLogLevel,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	WebsocketServer::~WebsocketServer()			
	{
		stop();
	}


	bool WebsocketServer::start(utility::ErrorState& errorState)
	{
		// Extract correct log levels
		uint32 log_level = computeWebSocketLogLevel(mLibraryLogLevel);
		uint32 alog_level = mLogConnectionUpdates ? websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload
			: websocketpp::log::alevel::fail;

		// Create the end point
		mEndpoint = std::make_unique<WebSocketServerEndPoint>(mPort,  log_level, alog_level);

		// Install handler
		mEndpoint->setMessageHandler(std::bind(
			&WebsocketServer::messageHandler, this,
			std::placeholders::_1, std::placeholders::_2
		));

		// Open and start listening
		if (!mEndpoint->open(errorState))
			return false;

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
		mEndpoint->send("who's your daddy now??", hdl, msg);
	}
}