#include "websocketmessage.h"

namespace nap
{

	WebSocketMessage::WebSocketMessage(WebSocketConnection connection, wspp::MessagePtr message) :
		mConnection(connection), mMessage(message)
	{

	}

}