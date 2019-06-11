#include "apiwebsockethandler.h"

// External Includes
#include <entity.h>
#include <nap/logger.h>

// nap::apiwebsockethandler run time class definition 
RTTI_BEGIN_CLASS(nap::APIWebSocketHandlerComponent)
	RTTI_PROPERTY("APIComponent",	&nap::APIWebSocketHandlerComponent::mAPIComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("TextComponent",	&nap::APIWebSocketHandlerComponent::mTextComponent, nap::rtti::EPropertyMetaData::Required)
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
	}

}