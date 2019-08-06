// Local Includes
#include "apiwebsocketserver.h"
#include "apiwebsocketservice.h"
#include "apiwebsocketevent.h"

// External Includes
#include <nap/logger.h>
#include <apiutils.h>
#include <nap/core.h>
#include <websocketservice.h>

// nap::websocketapiserver run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APIWebSocketServer)
	RTTI_CONSTRUCTOR(nap::APIWebSocketService&)
	RTTI_PROPERTY("Mode",		&nap::APIWebSocketServer::mMode,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Verbose",	&nap::APIWebSocketServer::mVerbose, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	APIWebSocketServer::APIWebSocketServer(APIWebSocketService& service) : 
		IWebSocketServer(service.getWebSocketService()), 
		mAPIService(&(service.getAPIService()))
	{

	}


	bool APIWebSocketServer::init(utility::ErrorState& errorState)
	{
		if (!IWebSocketServer::init(errorState))
			return false;

		mEndPoint->registerListener(*this);	
		return true;
	}


	void APIWebSocketServer::onDestroy()
	{
		mEndPoint->unregisterListener(*this);
	}


	bool APIWebSocketServer::send(nap::APIEventPtr apiEvent, const WebSocketConnection& connection, utility::ErrorState& error)
	{
		APIMessage msg(*apiEvent);
		std::string json;
		if (!msg.toJSON(json, error))
			return false;

		// Send msg
		if (!mEndPoint->send(connection, json, EWebSocketOPCode::Text, error))
			return false;
		return true;
	}


	void APIWebSocketServer::sendErrorReply(const WebSocketConnection& connection, nap::utility::ErrorState& error)
	{
		nap::utility::ErrorState snd_error;
		if (mVerbose)
			nap::Logger::warn("%s: %s", this->mID.c_str(), error.toString().c_str());

		if (!mEndPoint->send(connection, utility::stringFormat("ERROR: %s", error.toString().c_str()), EWebSocketOPCode::Text, snd_error))
			nap::Logger::error("%s: %s", this->mID.c_str(), snd_error.toString().c_str());
	}


	void APIWebSocketServer::onMessageReceived(const WebSocketConnection& connection, const WebSocketMessage& message)
	{
		// Add web-socket event
		switch(mMode)
		{
		case EWebSocketForwardMode::WebSocketEvent:
		{
			addEvent(std::make_unique<WebSocketMessageReceivedEvent>(connection, message));
			return;
		}
		case EWebSocketForwardMode::Both:
		{
			addEvent(std::make_unique<WebSocketMessageReceivedEvent>(connection, message));
			break;
		}
		default:
			break;
		}

		// Ensure it's a finalized message
		nap::utility::ErrorState error;
		if (!error.check(message.getFin(), "only finalized messages are accepted"))
		{
			if (mVerbose)
				nap::Logger::warn("%s: %s", mID.c_str(), error.toString().c_str());
			return;
		}

		// Make sure we're dealing with text
		if (!error.check(message.getCode() == EWebSocketOPCode::Text, "not a text message"))
		{
			if(mVerbose)
				nap::Logger::warn("%s: %s", mID.c_str(), error.toString().c_str());
			return;
		}

		// Perform extraction
		auto& factory = mService->getCore().getResourceManager()->getFactory();
		nap::rtti::DeserializeResult result;
		std::vector<APIMessage*> messages;
		if (!extractMessages(message.getPayload(), result, factory, messages, error))
		{
			sendErrorReply(connection, error);
			return;
		}

		// Create unique events and hand off to api service
		for (auto& apimsg : messages)
		{
			APIWebSocketEventPtr msg_event = apimsg->toEvent<APIWebSocketEvent>(connection, *this);
			if (!mAPIService->sendEvent(std::move(msg_event), &error))
			{
				sendErrorReply(connection, error);
			}
		}
	}


	void APIWebSocketServer::onConnectionOpened(const WebSocketConnection& connection)
	{
		// Add web-socket event
		switch (mMode)
		{
		case EWebSocketForwardMode::Both:
		case EWebSocketForwardMode::WebSocketEvent:
		{
			addEvent(std::make_unique<WebSocketConnectionOpenedEvent>(connection));
			break;
		}
		default:
			break;
		}
	}


	void APIWebSocketServer::onConnectionClosed(const WebSocketConnection& connection, int code, const std::string& reason)
	{
		// Add web-socket event
		switch (mMode)
		{
		case EWebSocketForwardMode::Both:
		case EWebSocketForwardMode::WebSocketEvent:
		{
			addEvent(std::make_unique<WebSocketConnectionClosedEvent>(connection, code, reason));
			break;
		}
		default:
			break;
		}
	}


	void APIWebSocketServer::onConnectionFailed(const WebSocketConnection& connection, int code, const std::string& reason)
	{
		// Add web-socket event
		switch (mMode)
		{
		case EWebSocketForwardMode::Both:
		case EWebSocketForwardMode::WebSocketEvent:
		{
			addEvent(std::make_unique<WebSocketConnectionFailedEvent>(connection, code, reason));
			break;
		}
		default:
			break;
		}
	}
}