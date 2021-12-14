/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "apiwebsocketclient.h"
#include "apiwebsocketevent.h"
#include "apiwebsocketservice.h"

// External Includes
#include <nap/logger.h>
#include <nap/core.h>
#include <apimessage.h>
#include <apiservice.h>
#include <apiutils.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APIWebSocketClient)
	RTTI_CONSTRUCTOR(nap::APIWebSocketService&)
	RTTI_PROPERTY("SendWebSocketEvents",	&nap::APIWebSocketClient::mSendWebSocketEvents,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Verbose",				&nap::APIWebSocketClient::mVerbose,				nap::rtti::EPropertyMetaData::Default)
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
		// Convert event to json 
		APIMessage msg(*apiEvent);
		std::string json;
		if (!msg.toJSON(json, error))
			return false;

		// Send message
		return mEndPoint->send(mConnection, json, EWebSocketOPCode::Text, error);
	}


	void APIWebSocketClient::onConnectionOpened()
	{
		// Add web-socket event
		if (mSendWebSocketEvents)
			addEvent(std::make_unique<WebSocketConnectionOpenedEvent>(mConnection));
	}


	void APIWebSocketClient::onConnectionClosed(int code, const std::string& reason)
	{
		// Add web-socket event
		if (mSendWebSocketEvents)
			addEvent(std::make_unique<WebSocketConnectionClosedEvent>(mConnection, code, reason));
	}


	void APIWebSocketClient::onConnectionFailed(int code, const std::string& reason)
	{
		// Add web-socket event
		if (mSendWebSocketEvents)
			addEvent(std::make_unique<WebSocketConnectionFailedEvent>(mConnection, code, reason));
	}


	void APIWebSocketClient::onMessageReceived(const WebSocketMessage& message)
	{
		// Add web-socket event
		if (mSendWebSocketEvents)
			addEvent(std::make_unique<WebSocketMessageReceivedEvent>(mConnection, message));

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
			if (mVerbose)
				nap::Logger::warn("%s: %s", mID.c_str(), error.toString().c_str());
			return;
		}

		// Perform extraction
		auto& factory = mService->getCore().getResourceManager()->getFactory();
		nap::rtti::DeserializeResult result;
		std::vector<APIMessage*> messages;
		if (!extractMessages(message.getPayload(), result, factory, messages, error))
		{
			if (mVerbose)
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
