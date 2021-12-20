/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "portalwebsocketserver.h"
#include "portalservice.h"
#include "portalutils.h"

// External Includes
#include <nap/logger.h>
#include <nap/core.h>
#include <apievent.h>
#include <apimessage.h>
#include <apiutils.h>

// nap::websocketapiserver run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PortalWebSocketServer)
	RTTI_CONSTRUCTOR(nap::PortalService&)
	RTTI_PROPERTY("SendWebSocketEvents",	&nap::PortalWebSocketServer::mSendWebSocketEvents,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Verbose",				&nap::PortalWebSocketServer::mVerbose,				nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	PortalWebSocketServer::PortalWebSocketServer(PortalService& service) :
		IWebSocketServer(service.getWebSocketService()),
		mService(&service)
	{

	}


	bool PortalWebSocketServer::init(utility::ErrorState& errorState)
	{
		if (!IWebSocketServer::init(errorState))
			return false;

		mService->registerServer(*this);
		return true;
	}


	void PortalWebSocketServer::onDestroy()
	{
		IWebSocketServer::onDestroy();
		mService->removeServer(*this);
	}


	bool PortalWebSocketServer::send(PortalEventPtr event, const WebSocketConnection& connection, utility::ErrorState& error)
	{
		// Convert event to json
		std::string json;
		if (!event->toAPIMessageJSON(json, error))
			return false;

		// Send message
		return mEndPoint->send(connection, json, EWebSocketOPCode::Text, error);
	}


	bool PortalWebSocketServer::broadcast(PortalEventPtr event, utility::ErrorState& error)
	{
		// Convert event to json
		std::string json;
		if (!event->toAPIMessageJSON(json, error))
			return false;

		// Broadcast message
		return mEndPoint->broadcast(json, EWebSocketOPCode::Text, error);
	}


	void PortalWebSocketServer::onConnectionOpened(const WebSocketConnection& connection)
	{
		// Add web-socket event
		if (mSendWebSocketEvents)
			addEvent(std::make_unique<WebSocketConnectionOpenedEvent>(connection));
	}


	void PortalWebSocketServer::onConnectionClosed(const WebSocketConnection& connection, int code, const std::string& reason)
	{
		// Add web-socket event
		if (mSendWebSocketEvents)
			addEvent(std::make_unique<WebSocketConnectionClosedEvent>(connection, code, reason));
	}


	void PortalWebSocketServer::onConnectionFailed(const WebSocketConnection& connection, int code, const std::string& reason)
	{
		// Add web-socket event
		if (mSendWebSocketEvents)
			addEvent(std::make_unique<WebSocketConnectionFailedEvent>(connection, code, reason));
	}


	void PortalWebSocketServer::onMessageReceived(const WebSocketConnection& connection, const WebSocketMessage& message)
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
		rtti::Factory& factory = mService->getCore().getResourceManager()->getFactory();
		rtti::DeserializeResult result;
		std::vector<APIMessage*> messages;
		if (!extractMessages(message.getPayload(), result, factory, messages, error))
		{
			if (mVerbose)
				nap::Logger::warn("%s: %s", mID.c_str(), error.toString().c_str());
			return;
		}

		// Get portal event header
		APIMessage* header_message = messages.front();
		APIEventPtr header_event = header_message->toEvent<APIEvent>();
		PortalEventHeader header;
		if (!extractPortalEventHeader(header_event, header, error))
		{
			if (mVerbose)
				nap::Logger::warn("%s: %s", mID.c_str(), error.toString().c_str());
			return;
		}

		// Create portal event
		PortalEventPtr portal_event = std::make_unique<PortalEvent>(header, connection);

		// Add API events that relate to portal items
		for (APIMessage* message : messages)
		{
			// Skip portal event header
			if (message == header_message)
				continue;

			APIEventPtr api_event = message->toEvent<APIEvent>();
			portal_event->addAPIEvent(std::move(api_event));
		}

		// Add portal event
		addPortalEvent(std::move(portal_event));
	}


	void PortalWebSocketServer::addPortalEvent(PortalEventPtr event)
	{
		std::lock_guard<std::mutex> lock(mPortalEventMutex);
		mPortalEvents.emplace(std::move(event));
	}


	void PortalWebSocketServer::consumePortalEvents(std::queue<PortalEventPtr>& outEvents)
	{
		// Swap events
		std::lock_guard<std::mutex> lock(mPortalEventMutex);
		outEvents.swap(mPortalEvents);

		// Clear current queue
		std::queue<PortalEventPtr> empty_queue;;
		mPortalEvents.swap(empty_queue);
	}
}
