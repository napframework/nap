// Local Includes
#include "websocketserver.h"
#include "websocketutils.h"

// External Includes
#include <nap/logger.h>

// nap::websocketserver run time class definition 
RTTI_BEGIN_CLASS(nap::WebSocketServer)
	RTTI_PROPERTY("EndPoint",				&nap::WebSocketServer::mEndPoint,					nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	WebSocketServer::~WebSocketServer()			
	{

	}


	bool WebSocketServer::init(utility::ErrorState& errorState)
	{
		mEndPoint->connectionClosed.connect(mConnectionClosedSlot);
		mEndPoint->connectionOpened.connect(mConnectionOpenedSlot);
		mEndPoint->messageReceived.connect(mMessageReceivedSlot);
		mEndPoint->connectionFailed.connect(mConnectionFailedSlot);

		return true;
	}


	void WebSocketServer::onMessageReceived(WebSocketMessage message)
	{
		addEvent(std::make_unique<WebSocketMessageReceivedEvent>(message));
	}


	void WebSocketServer::onConnectionClosed(WebSocketConnection connection)
	{
		addEvent(std::make_unique<WebSocketConnectionClosedEvent>(connection));
	}


	void WebSocketServer::onConnectionOpened(WebSocketConnection connection)
	{
		addEvent(std::make_unique<WebSocketConnectionOpenedEvent>(connection));
	}


	void WebSocketServer::onConnectionFailed(WebSocketConnection connection)
	{
		addEvent(std::make_unique<WebSocketConnectionFailedEvent>(connection));
	}


	void WebSocketServer::consumeEvents(std::queue<WebSocketEventPtr>& outEvents)
	{
	}


	void WebSocketServer::addEvent(WebSocketEventPtr newEvent)
	{
		std::lock_guard<std::mutex> lock(mEventMutex);
		mEvents.emplace(std::move(newEvent));
	}
}