/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "portalwebsocketcomponent.h"
#include "portalutils.h"
#include "portalevent.h"

 // External Includes
#include <entity.h>
#include <apiutils.h>
#include <apimessage.h>
#include <nap/core.h>
#include <nap/logger.h>

// nap::PortalWebSocketComponent run time class definition
RTTI_BEGIN_CLASS(nap::PortalWebSocketComponent)
RTTI_PROPERTY("Verbose", &nap::PortalWebSocketComponent::mVerbose, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::PortalWebSocketComponentInstance run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PortalWebSocketComponentInstance)
RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	PortalWebSocketComponentInstance::~PortalWebSocketComponentInstance()
	{
		// Stop listening to message received events
		messageReceived.disconnect(mMessageReceivedSlot);
	}


	bool PortalWebSocketComponentInstance::init(utility::ErrorState& errorState)
	{
		// Call WebSocket component init to register with the WebSocket service
		if (!WebSocketComponentInstance::init(errorState))
			return false;

		// Ensure the interface is of type nap::WebSocketServer
		if (!getInterface().get_type().is_derived_from<WebSocketServer>())
			return errorState.check(false, "%s: expects Interface to be of type nap::WebSocketServer", getComponent()->mID.c_str());

		// Store a pointer to the nap::WebSocketServer
		mWebSocketServer = &getInterface().as<WebSocketServer>();

		// Copy RTTI properties
		mVerbose = getComponent<PortalWebSocketComponent>()->mVerbose;

		// Store pointers to portal components
		getEntityInstance()->getComponentsOfType<PortalComponentInstance>(mPortalComponents);

		// Listen to message received events
		messageReceived.connect(mMessageReceivedSlot);

		return true;
	}


	void PortalWebSocketComponentInstance::onMessageReceived(const WebSocketMessageReceivedEvent& event)
	{
		nap::utility::ErrorState error;

		// Check the WebSocket message format before extracting API messages
		error.check(event.mMessage.getFin(), "WebSocket message is not finalized");
		error.check(event.mMessage.getCode() == EWebSocketOPCode::Text, "WebSocket message does not contain text");
		if (error.hasErrors())
		{
			if (mVerbose)
				nap::Logger::warn("%s: %s", mID.c_str(), error.toString().c_str());
			return;
		}

		// Extract API messages from WebSocket message
		nap::rtti::Factory& factory = getEntityInstance()->getCore()->getResourceManager()->getFactory();
		rtti::DeserializeResult result;
		std::vector<APIMessage*> api_messages;
		if (!extractMessages(event.mMessage.getPayload(), result, factory, api_messages, error))
		{
			if (mVerbose)
				nap::Logger::warn("%s: %s", mID.c_str(), error.toString().c_str());
			return;
		}

		// Get portal event header
		APIMessage* header_message = api_messages.front();
		APIEventPtr header_event = header_message->toEvent<APIEvent>();
		PortalEventHeader header;
		if (!extractPortalEventHeader(header_event, header, error))
		{
			if (mVerbose)
				nap::Logger::warn("%s: %s", mID.c_str(), error.toString().c_str());
			return;
		}

		// Create portal event
		PortalEventPtr portal_event = std::make_unique<PortalEvent>(header);

		// Add API events that relate to portal items
		for (APIMessage* message : api_messages)
		{
			// Skip portal event header
			if (message == header_message)
				continue;

			APIEventPtr api_event = message->toEvent<APIEvent>();
			portal_event->addAPIEvent(std::move(api_event));
		}

		// Send portal event to portal components
		if (!sendEvent(std::move(portal_event), error))
		{
			if (mVerbose)
				nap::Logger::warn("%s: %s", mID.c_str(), error.toString().c_str());
			return;
		}
	}


	bool PortalWebSocketComponentInstance::sendEvent(PortalEventPtr event, utility::ErrorState& error)
	{
		// Iterate over all the portal components to find the target
		for (const auto& portal : mPortalComponents)
			if (portal->getComponent()->mID == event->getPortalID())
				return portal->processEvent(std::move(event), error);

		// No portal component accepted the event
		return error.check(false, "%s: no portal component found with ID %s", getComponent()->mID.c_str(), event->getPortalID().c_str());
	}
}
