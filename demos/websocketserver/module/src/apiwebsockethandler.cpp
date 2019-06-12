#include "apiwebsockethandler.h"
#include "apiwebsocketevent.h"
#include "apiwebsocketserver.h"

// External Includes
#include <entity.h>
#include <nap/logger.h>

// nap::apiwebsockethandler run time class definition 
RTTI_BEGIN_CLASS(nap::APIWebSocketHandlerComponent)
	RTTI_PROPERTY("APIComponent",	&nap::APIWebSocketHandlerComponent::mAPIComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("TextComponent",	&nap::APIWebSocketHandlerComponent::mTextComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Server",			&nap::APIWebSocketHandlerComponent::mServer,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::apiwebsockethandlerInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APIWebSocketHandlerComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool APIWebSocketHandlerComponentInstance::init(utility::ErrorState& errorState)
	{
		const nap::APISignature* change_text_signature = mAPIComponent->findSignature("ChangeText");
		if (!errorState.check(change_text_signature != nullptr, "%s: unable to find 'ChangeText' signature", this->mID.c_str()))
		{
			errorState.fail("unable to install callback!");
			return false;
		}

		// Register callback
		mAPIComponent->registerCallback(*change_text_signature, mChangeTextSlot);

		// Store handle to server
		mServer = getComponent<APIWebSocketHandlerComponent>()->mServer.get();
		return true;
	}


	void APIWebSocketHandlerComponentInstance::update(double deltaTime)
	{

	}


	void APIWebSocketHandlerComponentInstance::onChangeText(const nap::APIEvent& apiEvent)
	{
		nap::utility::ErrorState error;
		if (!mTextComponent->setText(apiEvent[0].asString(), error))
		{
			nap::Logger::warn("unable to update text: %s", error.toString().c_str());
		}

		// Cast to api web-socket event
		const APIWebSocketEvent& ws_event = apiEvent.to<APIWebSocketEvent>();
		
		// Ensure that the interface (client / server) that created 
		// this event is the same as our handle to the server
		assert(mServer != &(ws_event.getInterface()));

		// Construct reply, copy over the unique id of the request.
		APIEventPtr text_changed_event = std::make_unique<APIEvent>("TextChanged", ws_event.getID());
		
		// Add as an argument the updated text.
		text_changed_event->addArgument<APIString>("newText", mTextComponent->getText());

		// Send to client using server
		if (!mServer->send(std::move(text_changed_event), ws_event.getConnection(), error))
		{
			nap::Logger::warn("unable to send reply: %s", error.toString().c_str());
		}
	}

}