// Local Includes
#include "apiwebsocketdispatcher.h"
#include "apiwebsocketservice.h"

// External Includes
#include <apimessage.h>
#include <nap/logger.h>

// nap::apiwebsocketdispatcher run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APIWebSocketDispatcher)
	RTTI_CONSTRUCTOR(nap::APIWebSocketService&)
	RTTI_PROPERTY("Server", &nap::APIWebSocketDispatcher::mServer, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	APIWebSocketDispatcher::~APIWebSocketDispatcher()			{ }


	APIWebSocketDispatcher::APIWebSocketDispatcher(APIWebSocketService& service) : 
		IAPIDispatcher(service.getAPIService()), mAPIWebSocketService(&service)
	{
	}


	bool APIWebSocketDispatcher::init(utility::ErrorState& errorState)
	{
		return IAPIDispatcher::init(errorState);
	}


	bool APIWebSocketDispatcher::onDispatch(const nap::APIEvent& apiEvent, nap::utility::ErrorState& error)
	{
		// Cast
		assert(apiEvent.get_type().is_derived_from(RTTI_OF(APIWebSocketEvent)));
		const APIWebSocketEvent& ws_event = static_cast<const APIWebSocketEvent&>(apiEvent);

		// Construct message from event and convert into json 
		// TODO: maybe thread this operation?
		APIMessage msg(ws_event);
		std::string json;
		if (!msg.toJSON(json, error))
		{
			sendErrorReply(ws_event.getConnection(), error);
			return false;
		}

		// Send msg
		if (!mServer->mEndPoint->send(ws_event.getConnection(), json, EWebSocketOPCode::Text, error))
			return false;

		return true;
	}


	void APIWebSocketDispatcher::sendErrorReply(const WebSocketConnection& connection, nap::utility::ErrorState& error)
	{
		nap::utility::ErrorState it_error;
		if (!mServer->mEndPoint->send(connection, utility::stringFormat("ERROR: %s", error.toString()), EWebSocketOPCode::Text, it_error))
			nap::Logger::error("%s: %s", this->mID.c_str(), it_error.toString().c_str());
	}
}