/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "portalservice.h"

// External Includes
#include <websocketcomponent.h>
#include <websocketevent.h>
#include <nap/signalslot.h>

namespace nap
{
	// Forward declares
	class PortalWebSocketComponentInstance;

	/**
	 * Resource part of the PortalWebSocketComponent.
	 * Transforms each WebSocket event it receives into a portal event and passes it to the portal service when successful.
	 */
	class NAPAPI PortalWebSocketComponent : public WebSocketComponent
	{
		RTTI_ENABLE(WebSocketComponent)
		DECLARE_COMPONENT(PortalWebSocketComponent, PortalWebSocketComponentInstance)

	public:

		bool mVerbose = true;	///< Property: 'Verbose' log WebSocket event to portal event conversion failures.
	};


	/*
	 * Instance part of the PortalWebSocketComponent,
	 * Transforms each WebSocket event it receives into a portal event and passes it to the portal service when successful.
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

		bool mVerbose = true;
		PortalService* mPortalService = nullptr;

		/**
		 * Slot that is connected to the 'messageReceived' signal
		 */
		Slot<const WebSocketMessageReceivedEvent&> mMessageReceivedSlot = { this, &PortalWebSocketComponentInstance::onMessageReceived };

		/**
		 * Called when the slot above receives a new message
		 * @param WebSocketMessageReceivedEvent the received WebSocket event
		 */
		void onMessageReceived(const WebSocketMessageReceivedEvent& event);
	};
}
