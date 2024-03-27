/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "websocketclient.h"
#include "websocketservice.h"

// External Includes
#include <nap/logger.h>

// nap::websocketclient run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IWebSocketClient)
	RTTI_PROPERTY("URI",		&nap::IWebSocketClient::mURI,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("EndPoint",	&nap::IWebSocketClient::mEndPoint,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Ticket",		&nap::IWebSocketClient::mTicket,	nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WebSocketClient)
	RTTI_CONSTRUCTOR(nap::WebSocketService&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// IWebSocketClient
	//////////////////////////////////////////////////////////////////////////

	IWebSocketClient::IWebSocketClient(WebSocketService& service) : WebSocketInterface(service)
	{ }


	bool IWebSocketClient::init(utility::ErrorState& errorState)
	{
		if (!WebSocketInterface::init(errorState))
			return false;

		return mEndPoint->registerClient(*this, errorState);
	}


	void IWebSocketClient::onDestroy()
	{
		mEndPoint->unregisterClient(*this);
	}


	bool IWebSocketClient::isConnected() const
	{
		return mOpen;
	}


	bool IWebSocketClient::reconnect(utility::ErrorState& error)
	{
		// First disconnect
		mEndPoint->unregisterClient(*this);

		// Now connect again
		return mEndPoint->registerClient(*this, error);
	}


	void IWebSocketClient::connectionOpened()
	{
		mOpen = true;
		onConnectionOpened();
	}


	void IWebSocketClient::connectionClosed(int code, const std::string& reason)
	{
		mOpen = false;
		onConnectionClosed(code, reason);
	}


	void IWebSocketClient::connectionFailed(int code, const std::string& reason)
	{
		mOpen = false;
		onConnectionFailed(code, reason);
	}


	void IWebSocketClient::messageReceived(const WebSocketMessage& msg)
	{
		onMessageReceived(msg);
	}


	//////////////////////////////////////////////////////////////////////////
	// WebSocketClient
	//////////////////////////////////////////////////////////////////////////

	WebSocketClient::WebSocketClient(WebSocketService& service) : IWebSocketClient(service)
	{ }


	bool WebSocketClient::send(const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error)
	{
		if (!error.check(isConnected(), "%s: client not connected to: %s", mID.c_str(), mURI.c_str()))
			return false;

		return mEndPoint->send(mConnection, message, code, error);
	}


	bool WebSocketClient::send(void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error)
	{
		if (!error.check(isConnected(), "%s: client not connected to: %s", mID.c_str(), mURI.c_str()))
			return false;

		return mEndPoint->send(mConnection, payload, length, code, error);
	}


	bool WebSocketClient::send(const WebSocketMessage& message, nap::utility::ErrorState& error)
	{
		if (!error.check(isConnected(), "%s: client not connected to: %s", mID.c_str(), mURI.c_str()))
			return false;

		return mEndPoint->send(mConnection, message.getPayload(), message.getCode(), error);
	}


	void WebSocketClient::onConnectionOpened()
	{
		addEvent(std::make_unique<WebSocketConnectionOpenedEvent>(mConnection));
	}


	void WebSocketClient::onConnectionClosed(int code, const std::string& reason)
	{
		addEvent(std::make_unique<WebSocketConnectionClosedEvent>(mConnection, code, reason));
	}


	void WebSocketClient::onConnectionFailed(int code, const std::string& reason)
	{
		addEvent(std::make_unique<WebSocketConnectionFailedEvent>(mConnection, code, reason));
	}


	void WebSocketClient::onMessageReceived(const WebSocketMessage& msg)
	{
		addEvent(std::make_unique<WebSocketMessageReceivedEvent>(mConnection, msg));
	}
}
