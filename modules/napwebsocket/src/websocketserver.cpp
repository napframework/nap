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

	WebSocketServer::WebSocketServer(WebSocketService& service) : mService(&service)
	{
		mService->registerServer(*this);
	}


	WebSocketServer::~WebSocketServer()
	{
		mService->removeServer(*this);
	}


	bool WebSocketServer::init(utility::ErrorState& errorState)
	{
		// Initialize base class
		if (!IWebSocketServer::init(errorState))
			return false;

		mConnectionClosed.setFunction(std::bind(&WebSocketServer::onConnectionClosed, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		mEndPoint->connectionClosed.connect(mConnectionClosed);
		
		mConnectionFailed.setFunction(std::bind(&WebSocketServer::onConnectionFailed, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		mEndPoint->connectionFailed.connect(mConnectionFailed);

		mConnectionOpened.setFunction(std::bind(&WebSocketServer::onConnectionOpened, this, std::placeholders::_1));
		mEndPoint->connectionOpened.connect(mConnectionOpened);

		mMessageReceived.setFunction(std::bind(&WebSocketServer::onMessageReceived, this, std::placeholders::_1, std::placeholders::_2));
		mEndPoint->messageReceived.connect(mMessageReceived);

		// Register the server with our service
		return true;
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

	void WebSocketServer::addEvent(WebSocketEventPtr newEvent)
	{
		std::lock_guard<std::mutex> lock(mEventMutex);
		mEvents.emplace(std::move(newEvent));
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


	void WebSocketServer::consumeEvents(std::queue<WebSocketEventPtr>& outEvents)
	{
		// Swap events
		std::lock_guard<std::mutex> lock(mEventMutex);
		outEvents.swap(mEvents);

		// Clear current queue
		std::queue<WebSocketEventPtr> empty_queue;;
		mEvents.swap(empty_queue);
	}


	bool IWebSocketServer::init(utility::ErrorState& errorState)
	{
		return true;
	}
}