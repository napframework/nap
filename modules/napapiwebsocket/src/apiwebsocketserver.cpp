// Local Includes
#include "apiwebsocketserver.h"
#include "apiwebsocketservice.h"
#include "apiwebsocketevent.h"

// External Includes
#include <nap/logger.h>
#include <apiutils.h>
#include <nap/core.h>

// nap::websocketapiserver run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APIWebSocketServer)
RTTI_CONSTRUCTOR(nap::APIWebSocketService&)
	RTTI_PROPERTY("Verbose", &nap::APIWebSocketServer::mVerbose, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{

	const std::string APIWebSocketServer::apiMessageHeaderName("NAP:MESSAGE");


	APIWebSocketServer::~APIWebSocketServer() { }


	APIWebSocketServer::APIWebSocketServer(APIWebSocketService& service) : mService(&service)
	{

	}


	bool APIWebSocketServer::init(utility::ErrorState& errorState)
	{
		if (!IWebSocketServer::init(errorState))
			return false;

		// Set function for when message slot is called
		mMessageReceived.setFunction(std::bind(&APIWebSocketServer::onMessageReceived, this, std::placeholders::_1, std::placeholders::_2));
		
		// Bind
		mEndPoint->messageReceived.connect(mMessageReceived);
		
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
		// Ensure it's a finalized message
		nap::utility::ErrorState error;
		if (!error.check(message.getFin(), "only finalized messages are accepted"))
		{
			if (mVerbose)
				nap::Logger::warn("%s: %s", mID.c_str(), error.toString().c_str());
			return;
		}

		// Ensure the message start with a substring
		// TODO: maybe include this in the header?
		if (!utility::startsWith(message.getPayload(), apiMessageHeaderName))
		{
			if (mVerbose)
				nap::Logger::warn("%s: received message without request header: %s", mID.c_str(), apiMessageHeaderName.c_str());
			return;
		}

		// Erase first part
		std::string json(message.getPayload());
		json.erase(0, apiMessageHeaderName.length());

		// Members necessary to extract messages
		auto& factory = mService->getCore().getResourceManager()->getFactory();
		nap::rtti::DeserializeResult result;
		std::vector<APIMessage*> messages;

		// Perform extraction
		if (!extractMessages(json, result, factory, messages, error))
		{
			sendErrorReply(connection, error);
			return;
		}

		// Create unique events and hand off to api service
		for (auto& apimsg : messages)
		{
			APIWebSocketEventPtr msg_event = apimsg->toEvent<APIWebSocketEvent>(connection);
			if (!mService->getAPIService().sendEvent(std::move(msg_event), &error))
				sendErrorReply(connection, error);
		}
	}
}