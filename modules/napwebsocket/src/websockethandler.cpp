#include "websockethandler.h"

// External Includes
#include <entity.h>
#include <nap/logger.h>

// nap::websockethandler run time class definition 
RTTI_BEGIN_CLASS(nap::WebSocketHandler)
	// Put additional properties here
RTTI_END_CLASS

// nap::websockethandlerInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WebSocketHandlerInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void WebSocketHandler::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(nap::WebSocketComponent));
	}


	bool WebSocketHandlerInstance::init(utility::ErrorState& errorState)
	{
		mWSComponent = getEntityInstance()->findComponent<nap::WebSocketComponentInstance>();
		assert(mWSComponent != nullptr);

		mWSComponent->connectionOpened.connect(mConnectionOpened);
		mWSComponent->connectionClosed.connect(mConnectionClosed);
		mWSComponent->messageReceived.connect(mMessageReceived);

		return true;
	}


	void WebSocketHandlerInstance::update(double deltaTime)
	{

	}


	void WebSocketHandlerInstance::onConnectionOpened(const WebSocketConnectionOpenedEvent& wsEvent)
	{
		std::cout << "Connection Opened" << "\n";
	}


	void WebSocketHandlerInstance::onConnectionClosed(const WebSocketConnectionClosedEvent& wsEvent)
	{
		nap::Logger::info("Connection Closed: %d, %s", wsEvent.mErrorCode, wsEvent.mReason.c_str());
	}


	void WebSocketHandlerInstance::onMessageReceived(const WebSocketMessageReceivedEvent& wsEvent)
	{
		nap::Logger::info("Message Received: %s", wsEvent.mMessage.getPayload().c_str());
		WebSocketServer& wsserver =  mWSComponent->getServer();
		nap::utility::ErrorState error;
		wsserver.send(wsEvent.mConnection, WebSocketMessage("ola baby!", wsEvent.mMessage.getCode()), error);
	}
}