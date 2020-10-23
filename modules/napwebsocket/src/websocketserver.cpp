/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "websocketserver.h"
#include "websocketutils.h"
#include "websocketservice.h"

// External Includes
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IWebSocketServer)
	RTTI_PROPERTY("EndPoint", &nap::IWebSocketServer::mEndPoint, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::websocketserver run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WebSocketServer)
	RTTI_CONSTRUCTOR(nap::WebSocketService&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{

	WebSocketServer::WebSocketServer(WebSocketService& service) : IWebSocketServer(service)
	{
	}


	bool WebSocketServer::init(utility::ErrorState& errorState)
	{
		// Initialize base class
		if (!IWebSocketServer::init(errorState))
			return false;

		mEndPoint->registerListener(*this);
		return true;
	}


	void WebSocketServer::onDestroy()
	{
		mEndPoint->unregisterListener(*this);
	}


	bool WebSocketServer::send(const WebSocketConnection& connection, void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error)
	{
		return mEndPoint->send(connection, payload, length, code, error);
	}


	bool WebSocketServer::send(const WebSocketConnection& connection, const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error)
	{
		return mEndPoint->send(connection, message, code, error);
	}


	bool WebSocketServer::send(const WebSocketConnection& connection, const WebSocketMessage& message, nap::utility::ErrorState& error)
	{
		return mEndPoint->send(connection, message.getPayload(), message.getCode(), error);
	}


	void WebSocketServer::onConnectionOpened(const WebSocketConnection& connection)
	{
		addEvent(std::make_unique<WebSocketConnectionOpenedEvent>(connection));
	}


	void WebSocketServer::onConnectionClosed(const WebSocketConnection& connection, int code, const std::string& reason)
	{
		addEvent(std::make_unique<WebSocketConnectionClosedEvent>(connection, code, reason));
	}


	void WebSocketServer::onConnectionFailed(const WebSocketConnection& connection, int code, const std::string& reason)
	{
		addEvent(std::make_unique<WebSocketConnectionFailedEvent>(connection, code, reason));
	}


	void WebSocketServer::onMessageReceived(const WebSocketConnection& connection, const WebSocketMessage& message)
	{
		addEvent(std::make_unique<WebSocketMessageReceivedEvent>(connection, message));
	}


	IWebSocketServer::IWebSocketServer(WebSocketService& service) : WebSocketInterface(service)
	{

	}

	bool IWebSocketServer::init(utility::ErrorState& errorState)
	{
		return WebSocketInterface::init(errorState);
	}
}