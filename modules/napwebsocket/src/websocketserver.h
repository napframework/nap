#pragma once

// Local Includes
#include "websocketevents.h"

// External Includes
#include <queue>
#include <rtti/factory.h>
#include <nap/resource.h>

namespace nap
{
	class WebSocketService;

	/**
	 * Interface for a web-socket server that listens to incoming web-socket events.
	 * Override the onEventReceived() method to provide a handler for newly received web socket events.
	 * The WebSocketServerEndPoint can have one or multiple links to an IWebSocketServer
	 */
	class NAPAPI IWebSocketServer : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		// Destructor
		virtual ~IWebSocketServer() override;

		/**
		 * If this server accepts the given web socket event. By default all web socket events are accepted.
		 * @param eventType the web socket event type
		 * @return if this server accepts the given web socket event
		 */
		virtual bool accepts(rtti::TypeInfo eventType)						{ return true; }

		/**
		 * Called by the web socket server end point. Calls onEventReceived().
		 * @param event the event to add, note that this receiver will take ownership of the event
		 */
		void addEvent(WebSocketEventPtr newEvent);

	protected:
		/**
		 * Called by the web socket server end point when a new event is received and the event is accepted by this server.
		 * Override this method to react to specific web socket events.
		 * @param newEvent the event that is received
		 */
		virtual void onEventReceived(WebSocketEventPtr newEvent) = 0;
	};


	//////////////////////////////////////////////////////////////////////////
	// WebSocketServer
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Allows for receiving and responding to messages over a web socket. Implements the IWebSocketServer interface.
	 * The server receives web socket events that can be interpreted by the running application.
	 * Events are received on a separate thread and consumed on the main thread by the WebSocketService.
	 */
	class NAPAPI WebSocketServer : public IWebSocketServer
	{
		friend class WebSocketService;
		RTTI_ENABLE(IWebSocketServer)
	public:

		// Constructor used by factory
		WebSocketServer(WebSocketService& service);

		// Destructor
		virtual ~WebSocketServer();

		/**
		 * Initializes the server.
		 * @param errorState contains the error if the server can't be started
		 * @return if the server started
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	protected:
		/**
		 * Called when the end point receives a new event.
		 * Adds the event to the list of events to be processed on the main thread.
		 * @param newEvent the web-socket event.
		 */
		virtual void onEventReceived(WebSocketEventPtr newEvent) override;

	private:
		// Queue that holds all the consumed events
		std::queue<WebSocketEventPtr> mEvents;

		// Mutex associated with setting / getting events
		std::mutex	mEventMutex;

		/**
		 * Consumes all received web-socket events and moves them to outEvents
		 * Calling this will clear the internal queue and transfers ownership of the events to the caller
		 * @param outEvents will hold the transferred web-socket events
		 */
		void consumeEvents(std::queue<WebSocketEventPtr>& outEvents);

		// Handle to the web socket service
		WebSocketService* mService = nullptr;
	};

	// Object creator used for constructing the the OSC receiver
	using WebSocketServerObjectCreator = rtti::ObjectCreator<WebSocketServer, WebSocketService>;
}
