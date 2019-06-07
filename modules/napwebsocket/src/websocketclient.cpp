#include "websocketclient.h"
#include <nap/logger.h>

// nap::websocketclient run time class definition 
RTTI_BEGIN_CLASS(nap::IWebSocketClient)
	RTTI_PROPERTY("EndPoint",	&nap::IWebSocketClient::mEndPoint,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("URI",		&nap::IWebSocketClient::mURI,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	IWebSocketClient::~IWebSocketClient()			
	{
		disconnect(mConnection);
	}


	bool IWebSocketClient::init(utility::ErrorState& errorState)
	{
		if (!mEndPoint->connect(*this, errorState))
			return false;
		return true;
	}


	bool IWebSocketClient::isConnected() const
	{
		return true;
	}

}