#include "websocketmessage.h"

namespace nap
{
	WebSocketMessage::WebSocketMessage(wspp::MessagePtr message) : mMessage(message)
	{

	}


	const std::string& WebSocketMessage::getPayload() const
	{
		return mMessage->get_payload();
	}


	nap::EWebSocketOPCode WebSocketMessage::getCode() const
	{
		return static_cast<EWebSocketOPCode>(mMessage->get_opcode());
	}


	bool WebSocketMessage::getFin() const
	{
		return mMessage->get_fin();
	}


	bool WebSocketMessage::getCompressed() const
	{
		return mMessage->get_compressed();
	}

}