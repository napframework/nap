#include "apiwebsocketclient.h"
#include "websocketservice.h"

#include <nap/logger.h>
#include <nap/core.h>
#include <apimessage.h>
#include <apiutils.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APIWebSocketClient)
	RTTI_CONSTRUCTOR(nap::WebSocketService&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	APIWebSocketClient::APIWebSocketClient(WebSocketService& service) : IWebSocketClient(service)
	{
	}


	APIWebSocketClient::~APIWebSocketClient()
	{

	}


	bool APIWebSocketClient::init(utility::ErrorState& errorState)
	{
		if (!IWebSocketClient::init(errorState))
			return false;
		return true;
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
		addEvent(std::make_unique<WebSocketConnectionOpenedEvent>(mConnection));
	}


	void APIWebSocketClient::onConnectionClosed(int code, const std::string& reason)
	{
		addEvent(std::make_unique<WebSocketConnectionClosedEvent>(mConnection, code, reason));
	}


	void APIWebSocketClient::onConnectionFailed(int code, const std::string& reason)
	{
		addEvent(std::make_unique<WebSocketConnectionFailedEvent>(mConnection, code, reason));
	}


	void APIWebSocketClient::onMessageReceived(const WebSocketMessage& msg)
	{
		addEvent(std::make_unique<WebSocketMessageReceivedEvent>(mConnection, msg));
	}
}