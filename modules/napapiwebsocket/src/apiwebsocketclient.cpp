/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "apiwebsocketclient.h"
#include "apiwebsocketservice.h"
#include "websocketservice.h"

// External Includes
#include <nap/logger.h>
#include <nap/core.h>
#include <apimessage.h>
#include <apiutils.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APIWebSocketClient)
	RTTI_CONSTRUCTOR(nap::APIWebSocketService&)
	RTTI_PROPERTY("Mode",		&nap::APIWebSocketClient::mMode,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Verbose",	&nap::APIWebSocketClient::mVerbose, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	APIWebSocketClient::APIWebSocketClient(APIWebSocketService& service) : 
		IWebSocketClient(service.getWebSocketService()),
		mAPIService(&(service.getAPIService()))
	{

	}


	bool APIWebSocketClient::send(APIEventPtr apiEvent, utility::ErrorState& error)
	{
		// Construct message from event and convert into json 
		APIMessage msg(*apiEvent);
		std::string json;
		if (!msg.toJSON(json, error))
			return false;

		// Send msg
		if (!mEndPoint->send(mConnection, json, EWebSocketOPCode::Text, error))
			return false;
		return true;
	}


	bool APIWebSocketClient::convert(const WebSocketMessage& message, std::vector<APIEventPtr>& outEvents, utility::ErrorState& error)
	{
		// Make sure we're dealing with text
		if (!error.check(message.getCode() == EWebSocketOPCode::Text, "not a text message"))
			return false;
		
		// Members necessary to extract messages
		auto& factory = mService->getCore().getResourceManager()->getFactory();
		nap::rtti::DeserializeResult result;
		std::vector<APIMessage*> messages;

		// Perform extraction
		if (!extractMessages(message.getPayload(), result, factory, messages, error))
			return false;

		// Convert to events
		outEvents.clear();
		for (auto& message : messages)
			outEvents.emplace_back(message->toEvent<APIEvent>());
		return true;
	}


	void APIWebSocketClient::onConnectionOpened()
	{
		// Add web-socket event
		switch (mMode)
		{
		case EWebSocketForwardMode::Both:
		case EWebSocketForwardMode::WebSocketEvent:
		{
			addEvent(std::make_unique<WebSocketConnectionOpenedEvent>(mConnection));
			break;
		}
		default:
			break;
		}
	}


	void APIWebSocketClient::onConnectionClosed(int code, const std::string& reason)
	{
		// Add web-socket event
		switch (mMode)
		{
		case EWebSocketForwardMode::Both:
		case EWebSocketForwardMode::WebSocketEvent:
		{
			addEvent(std::make_unique<WebSocketConnectionClosedEvent>(mConnection, code, reason));
			break;
		}
		default:
			break;
		}
	}


	void APIWebSocketClient::onConnectionFailed(int code, const std::string& reason)
	{
		// Add web-socket event
		switch (mMode)
		{
		case EWebSocketForwardMode::Both:
		case EWebSocketForwardMode::WebSocketEvent:
		{
			addEvent(std::make_unique<WebSocketConnectionFailedEvent>(mConnection, code, reason));
			break;
		}
		default:
			break;
		}
	}


	void APIWebSocketClient::onMessageReceived(const WebSocketMessage& msg)
	{
		// Add web-socket event
		switch (mMode)
		{
		case EWebSocketForwardMode::WebSocketEvent:
		{
			addEvent(std::make_unique<WebSocketMessageReceivedEvent>(mConnection, msg));
			return;
		}
		case EWebSocketForwardMode::Both:
		{
			addEvent(std::make_unique<WebSocketMessageReceivedEvent>(mConnection, msg));
			break;
		}
		default:
			break;
		}

		// Ensure it's a finalized message
		nap::utility::ErrorState error;
		if (!error.check(msg.getFin(), "only finalized messages are accepted"))
		{
			if (mVerbose)
				nap::Logger::warn("%s: %s", mID.c_str(), error.toString().c_str());
			return;
		}

		// Make sure we're dealing with text
		if (!error.check(msg.getCode() == EWebSocketOPCode::Text, "not a text message"))
		{
			if (mVerbose)
				nap::Logger::warn("%s: %s", mID.c_str(), error.toString().c_str());
			return;
		}

		// Perform extraction
		auto& factory = mService->getCore().getResourceManager()->getFactory();
		nap::rtti::DeserializeResult result;
		std::vector<APIMessage*> messages;
		if (!extractMessages(msg.getPayload(), result, factory, messages, error))
		{
			if(mVerbose)
				nap::Logger::warn("%s: %s", this->mID.c_str(), error.toString().c_str());
			return;
		}

		// Create unique events and hand off to api service
		for (auto& apimsg : messages)
		{
			APIWebSocketEventPtr msg_event = apimsg->toEvent<APIWebSocketEvent>(mConnection, *this);
			if (!mAPIService->sendEvent(std::move(msg_event), &error))
			{
				if (mVerbose)
					nap::Logger::warn("%s: %s", this->mID.c_str(), error.toString().c_str());
				return;
			}
		}
	}
}
