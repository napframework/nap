/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "websocketinterface.h"

// External Includes
#include <component.h>
#include <nap/signalslot.h>
#include <nap/resourceptr.h>

namespace nap
{
	// Forward Declares
	class WebSocketComponentInstance;

	/**
	 * Resource part of the WebSocketComponent, receives web-socket client and server events.
	 * Listen to the various signals (nap::WebSocketComponentInstance) to receive websocket events in your application.
	 * The events are received on the main application thread. This object ensures thread-safety.
	 * Only web-socket events that originated from the given interface (client or server) are accepted.
	 */
	class NAPAPI WebSocketComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(WebSocketComponent, WebSocketComponentInstance)
	public:
		nap::ResourcePtr<WebSocketInterface> mInterface;			///< Property: "Interface" the web-socket client or server this component listens to for events.
	};


	/**
	 * Instance part of the WebSocketComponent, receives web-socket client and server events.
	 * Listen to the various signals to receive web-socket events in your application.
	 * The events are received on the main application thread. This object ensures thread-safety.
	 * Only web-socket events that originated from the given interface (client or server) are accepted.
	 */
	class NAPAPI WebSocketComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		/**
		 * Constructor.
		 * @param entity the entity this component belongs to.
		 * @param resource the original nap::WebSocketComponent 
		 */
		WebSocketComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		// Destructor
		virtual ~WebSocketComponentInstance();

		/**
		 * Initialize this instance of the nap::WebSocketComponent.
		 * @param errorState holds the error message when initialization fails.
		 * @return if the component initialized successfully.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the client or server this component receives events from.
		 */
		const WebSocketInterface& getInterface() const					{ return *mInterface; }

		/**
		 * @return the client or server this component receives events from.
		 */
		WebSocketInterface& getInterface()								{ return *mInterface; }

		/**
		 * Occurs when a new connection to a client or server is established.
		 */
		nap::Signal<const WebSocketConnectionOpenedEvent&> connectionOpened;
		
		/**
		 * Occurs when a connection to a client or server is closed.
		 */
		nap::Signal<const WebSocketConnectionClosedEvent&> connectionClosed;
		
		/**
		 * Occurs when a connection to a client or server failed to establish.
		 */
		nap::Signal<const WebSocketConnectionFailedEvent&> connectionFailed;

		/**
		 * Occurs when a new message from a client or server is received.
		 */
		nap::Signal<const WebSocketMessageReceivedEvent&> messageReceived;

	private:
		WebSocketInterface* mInterface  = nullptr;		///< Handle to the web-socket client or server.
		WebSocketService* mService = nullptr;			///< Handle to the web-socket service.
	};
}
