#include "apiwebsocketserver.h"
#include "apiwebsocketservice.h"

// nap::websocketapiserver run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APIWebSocketServer)
	RTTI_CONSTRUCTOR(nap::APIWebSocketService&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	APIWebSocketServer::~APIWebSocketServer()			{ }


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


	void APIWebSocketServer::onMessageReceived(WebSocketConnection connection, WebSocketMessage message)
	{
		std::cout << "Hey there from the api server!" << "\n";
	}
}