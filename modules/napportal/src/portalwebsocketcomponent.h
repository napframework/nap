/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "portalcomponent.h"

// External Includes
#include <websocketcomponent.h>
#include <websocketevent.h>
#include <websocketserver.h>
#include <nap/signalslot.h>

namespace nap
{
	// Forward declares
	class PortalWebSocketComponentInstance;

	/**
	 * Resource part of the PortalWebSocketComponent.
	 * Transforms each WebSocket event it receives into a portal event and sends it to the target portal component when present in the same entity.
	 */
	class NAPAPI PortalWebSocketComponent : public WebSocketComponent
	{
		RTTI_ENABLE(WebSocketComponent)
		DECLARE_COMPONENT(PortalWebSocketComponent, PortalWebSocketComponentInstance)

	public:
		/**
		* Retrieves portal components on initialization.
		*/
		void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override
		{
			components.push_back(RTTI_OF(PortalComponent));
		}

		bool mVerbose = true;	///< Property: 'Verbose' log WebSocket event to portal event conversion failures.
	};


	/*
	 * Instance part of the PortalWebSocketComponent,
	 * Transforms each WebSocket event it receives into a portal event and sends it to the target portal component when present in the same entity.
	 */
	class NAPAPI PortalWebSocketComponentInstance : public WebSocketComponentInstance
	{
		RTTI_ENABLE(WebSocketComponentInstance)

	public:

		/**
		 * Constructor
		 */
		PortalWebSocketComponentInstance(EntityInstance& entity, Component& resource) :
			WebSocketComponentInstance(entity, resource) { }

		/**
		 * Destructor
		 */
		virtual ~PortalWebSocketComponentInstance();

		/**
		 * Initializes the portal WebSocket component instance
		 * @param errorState should hold the error message when initialization fails
		 * @return if the component initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	private:

		/**
		 * Called when the slot above receives a new message
		 * @param WebSocketMessageReceivedEvent the received WebSocket event
		 */
		void onMessageReceived(const WebSocketMessageReceivedEvent& event);

		/**
		 * Attempts to send the portal event to a portal component in the same entity
		 * @param event the portal event to send to a portal component
		 * @param error should hold the error message when sending fails
		 * @return if the event was sent successfully
		 */
		bool sendEvent(PortalEventPtr event, utility::ErrorState& error); 

		// WebSocket server that is selected as interface
		WebSocketServer* mWebSocketServer = nullptr;

		// Log WebSocket event to portal event conversion failures
		bool mVerbose = true;

		// Store pointers to portal components on init
		std::vector<PortalComponentInstance*> mPortalComponents;

		// Slot that is connected to the 'messageReceived' signal
		Slot<const WebSocketMessageReceivedEvent&> mMessageReceivedSlot = { this, &PortalWebSocketComponentInstance::onMessageReceived };
	};
}
