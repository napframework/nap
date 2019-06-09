#include "websocketclient.h"
#include <nap/logger.h>

// nap::websocketclient run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IWebSocketClient)
	RTTI_PROPERTY("EndPoint",	&nap::IWebSocketClient::mEndPoint,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("URI",		&nap::IWebSocketClient::mURI,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::WebSocketClient)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	IWebSocketClient::~IWebSocketClient()			
	{
		destroyed(*this);
	}


	bool IWebSocketClient::init(utility::ErrorState& errorState)
	{
		if (!mEndPoint->registerClient(*this, errorState))
			return false;
		return true;
	}


	bool IWebSocketClient::isOpen() const
	{
		return mOpen;
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


	bool WebSocketClient::init(utility::ErrorState& errorState)
	{
		if (!IWebSocketClient::init(errorState))
			return false;
		return true;
	}


	bool WebSocketClient::send(const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error)
	{
		if (!error.check(isOpen(), "%s: client not connected to: %s", mID.c_str(), mURI.c_str()))
			return false;

		if (!mEndPoint->send(mConnection, message, code, error))
			return false;

		return true;
	}


	bool WebSocketClient::send(void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error)
	{
		if (!error.check(isOpen(), "%s: client not connected to: %s", mID.c_str(), mURI.c_str()))
			return false;

		if (!mEndPoint->send(mConnection, payload, length, code, error))
			return false;

		return true;
	}


	bool WebSocketClient::send(const WebSocketMessage& message, nap::utility::ErrorState& error)
	{
		if (!error.check(isOpen(), "%s: client not connected to: %s", mID.c_str(), mURI.c_str()))
			return false;

		if (!mEndPoint->send(mConnection, message.getPayload(), message.getCode(), error))
			return false;

		return true;
	}

	void WebSocketClient::onConnectionOpened()
	{
		nap::utility::ErrorState error;
		if (!send("hi there!", EWebSocketOPCode::Text, error))
			assert(false);
	}


	void WebSocketClient::onConnectionClosed(int code, const std::string& reason)
	{

	}


	void WebSocketClient::onConnectionFailed(int code, const std::string& reason)
	{

	}

}