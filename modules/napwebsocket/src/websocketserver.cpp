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

		// Register the server with our service
		return true;
	}


	void WebSocketServer::onEventReceived(WebSocketEventPtr newEvent)
	{
		std::lock_guard<std::mutex> lock(mEventMutex);
		mEvents.emplace(std::move(newEvent));
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


	void IWebSocketServer::addEvent(WebSocketEventPtr newEvent)
	{
		onEventReceived(std::move(newEvent));
	}


	IWebSocketServer::~IWebSocketServer()
	{
		mEndPoint->removeServer(*this);
	}


	bool IWebSocketServer::init(utility::ErrorState& errorState)
	{
		mEndPoint->registerServer(*this);
		return true;
	}
}