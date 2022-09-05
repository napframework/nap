/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "apiwebsocketserver.h"
#include "apiwebsocketservice.h"
#include "apiwebsocketevent.h"

// External Includes
#include <nap/logger.h>
#include <nap/core.h>
#include <apimessage.h>
#include <apiservice.h>
#include <apiutils.h>

// nap::websocketapiserver run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APIWebSocketServer)
	RTTI_CONSTRUCTOR(nap::APIWebSocketService&)
	RTTI_PROPERTY("SendWebSocketEvents",	&nap::APIWebSocketServer::mSendWebSocketEvents,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Verbose",				&nap::APIWebSocketServer::mVerbose,				nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	APIWebSocketServer::APIWebSocketServer(APIWebSocketService& service) : 
		IWebSocketServer(service.getWebSocketService()), 
		mAPIService(&(service.getAPIService()))
	{

	}


	bool APIWebSocketServer::send(nap::APIEventPtr apiEvent, const WebSocketConnection& connection, utility::ErrorState& error)
	{
		// Convert event to json 
		APIMessage msg(*apiEvent);
		std::string json;
		if (!msg.toJSON(json, error))
			return false;

		// Send message
		return mEndPoint->send(connection, json, EWebSocketOPCode::Text, error);
	}


	bool APIWebSocketServer::broadcast(nap::APIEventPtr apiEvent, nap::utility::ErrorState& error)
	{
		// Convert event to json 
		APIMessage msg(*apiEvent);
		std::string json;
		if (!msg.toJSON(json, error))
			return false;

		// Broadcast message
		return mEndPoint->broadcast(json, EWebSocketOPCode::Text, error);
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
		if (mSendWebSocketEvents)
			addEvent(std::make_unique<WebSocketMessageReceivedEvent>(connection, message));

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
				return;
			}
		}
	}


	void APIWebSocketServer::onConnectionOpened(const WebSocketConnection& connection)
	{
		// Add web-socket event
		if (mSendWebSocketEvents)
			addEvent(std::make_unique<WebSocketConnectionOpenedEvent>(connection));
	}


	void APIWebSocketServer::onConnectionClosed(const WebSocketConnection& connection, int code, const std::string& reason)
	{
		// Add web-socket event
		if (mSendWebSocketEvents)
			addEvent(std::make_unique<WebSocketConnectionClosedEvent>(connection, code, reason));
	}


	void APIWebSocketServer::onConnectionFailed(const WebSocketConnection& connection, int code, const std::string& reason)
	{
		// Add web-socket event
		if (mSendWebSocketEvents)
			addEvent(std::make_unique<WebSocketConnectionFailedEvent>(connection, code, reason));
	}
}
