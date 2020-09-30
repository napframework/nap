#include "websocketmessage.h"

#include <nap/logger.h>

namespace nap
{
	WebSocketMessage::WebSocketMessage(wspp::MessagePtr message)
	{
		mMessage = message->get_payload();
		mCode = static_cast<EWebSocketOPCode>(message->get_opcode());
		mFin = message->get_fin();
	}


	WebSocketMessage::WebSocketMessage(const std::string& message, EWebSocketOPCode code, bool fin) :
		mMessage(message),
		mCode(code),
		mFin(fin)
	{

	}

	WebSocketMessage::WebSocketMessage(std::string&& message, EWebSocketOPCode code, bool fin) :
		mMessage(std::move(message)),
		mCode(code),
		mFin(fin)
	{
	}

	WebSocketMessage::WebSocketMessage(WebSocketMessage&& other) :
		mMessage(std::move(other.mMessage)),
		mCode(other.mCode),
		mFin(other.mFin)
	{
	}


	const std::string& WebSocketMessage::getPayload() const
	{
		return mMessage;
	}


	nap::EWebSocketOPCode WebSocketMessage::getCode() const
	{
		return mCode;
	}


	bool WebSocketMessage::getFin() const
	{
		return mFin;
	}


	void WebSocketMessage::appendPayload(const std::string& payload)
	{
		mMessage.append(payload);
	}


	WebSocketMessage& WebSocketMessage::operator=(WebSocketMessage&& other)
	{
		mMessage = std::move(other.mMessage);
		mCode = other.mCode;
		mFin = other.mFin;
		return *this;
	}

}