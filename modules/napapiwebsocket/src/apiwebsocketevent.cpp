#include "apiwebsocketevent.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APIWebSocketEvent)
	RTTI_CONSTRUCTOR(const std::string&, const nap::WebSocketConnection&)
	RTTI_CONSTRUCTOR(const std::string&, const std::string&, const nap::WebSocketConnection&)
RTTI_END_CLASS

namespace nap
{
	APIWebSocketEvent::APIWebSocketEvent(const std::string& name, const WebSocketConnection& connection) :
		APIEvent(name), mConnection(connection)
	{

	}

	APIWebSocketEvent::APIWebSocketEvent(std::string&& name, const WebSocketConnection& connection) :
		APIEvent(std::move(name)), mConnection(connection)
	{

	}


	APIWebSocketEvent::APIWebSocketEvent(const std::string& name, const std::string& id, const WebSocketConnection& connection) :
		APIEvent(name, id), mConnection(connection)
	{

	}


	APIWebSocketEvent::APIWebSocketEvent(std::string&& name, std::string&& id, const WebSocketConnection& connection) :
		APIEvent(std::move(name), std::move(id)), mConnection(connection)
	{

	}

}