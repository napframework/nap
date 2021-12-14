/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "portalevent.h"

// External Includes
#include <websocketserver.h>
#include <queue>
#include <mutex>

namespace nap
{
	// Forward Declares
	class PortalService;

	/**
	 * Portal WebSocket server implementation for sending and receiving API messages formatted as portal events.
	 * Implements the IWebSocketServer interface.
	 */
	class NAPAPI PortalWebSocketServer : public IWebSocketServer
	{
		friend class PortalService;
		RTTI_ENABLE(IWebSocketServer)
	public:
		/**
		 * Constructor
		 * @param service handle to the portal service
		 */
		PortalWebSocketServer(PortalService& service);

		/**
		 * Sends a portal event to a client.
		 * @param event the portal event to send to the client
		 * @param connection the client connection handle
		 * @param error contains the error if sending fails
		 * @return if the message was sent successfully
		 */
		bool send(PortalEvent event, const WebSocketConnection& connection, utility::ErrorState& error);

		/**
		 * Broadcasts a portal event to all connected clients.
		 * @param event the portal event to broadcast to the clients
		 * @param error contains the error if broadcasting fails
		 * @return if the message was broadcast successfully
		 */
		bool broadcast(PortalEvent event, utility::ErrorState& error);

		bool mSendWebSocketEvents = true;		///< Property: 'SendWebSocketEvents' send events to WebSocket service as well as portal service
		bool mVerbose = true;					///< Property: 'Verbose' log server message to portal event conversion failures.

	private:

		// Called by web-socket server endpoint when a client connection opened
		virtual void onConnectionOpened(const WebSocketConnection& connection) override;

		// Called by web-socket server endpoint when a client connection closed
		virtual void onConnectionClosed(const WebSocketConnection& connection, int code, const std::string& reason) override;

		// Called by web-socket server endpoint when a client connection failed to extablish
		virtual void onConnectionFailed(const WebSocketConnection& connection, int code, const std::string& reason) override;

		// Called by web-socket server endpoint when a new message is received
		virtual void onMessageReceived(const WebSocketConnection& connection, const WebSocketMessage& message) override;

		/**
		 * Adds a new portal event to the queue
		 * @param event the new event to add to the queue
		 */
		void addPortalEvent(PortalEventPtr event);

		/**
		 * Consumes all received portal  events and moves them to outEvents
		 * Calling this will clear the internal queue and transfers ownership of the events to the caller
		 * @param outEvents will hold the transferred portal events
		 */
		void consumePortalEvents(std::queue<PortalEventPtr>& outEvents);

		// Queue that holds all the received portal events
		std::queue<PortalEventPtr> mPortalEvents;

		// Mutex associated with setting / getting portal events
		std::mutex mPortalEventMutex;
	};

	// Object creator used for constructing the portal WebSocket server
	using PortalWebSocketServerObjectCreator = rtti::ObjectCreator<PortalWebSocketServer, PortalService>;
}
