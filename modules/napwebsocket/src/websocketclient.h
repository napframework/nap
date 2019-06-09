#pragma once

// Local Includes
#include "websocketclientendpoint.h"
#include "websocketevent.h"

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <nap/signalslot.h>
#include <rtti/factory.h>
#include <atomic>

namespace nap
{
	class WebSocketService;

	/**
	 * websocketclient
	 */
	class NAPAPI IWebSocketClient : public Resource
	{
		friend class WebSocketClientEndPoint;
		RTTI_ENABLE(Resource)
	public:
		// Destructor
		virtual ~IWebSocketClient();

		/**
		* Registers the web-socket client interface with the endpoint.
		* @param errorState contains the error message when initialization fails
		* @return if initialization succeeded.
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return if the client is connected
		 */
		bool isOpen() const;

		ResourcePtr<WebSocketClientEndPoint> mEndPoint;					///< Property: 'EndPoint' the client endpoint that manages all connections.
		std::string mURI;												///< Property: "UIR" Server URI to open connection to.

	protected:
		WebSocketConnection mConnection;								///< Web-socket connection

		virtual void onConnectionOpened() = 0;
		virtual void onConnectionClosed(int code, const std::string& reason) = 0;
		virtual void onConnectionFailed(int code, const std::string& reason) = 0;
		virtual void onMessageReceived(const WebSocketMessage& msg);

	private:
		// Called by web-socket client endpoint when the connection is opened
		void connectionOpened();
		// Called by web-socket client endpoint when the connection is closed
		void connectionClosed(int code, const std::string& reason);
		// Called by web-socket client endpoint when the connection failed to establish
		void connectionFailed(int code, const std::string& reason);
		// Called by web-socket client endpoint when a new message is received
		void messageReceived(const WebSocketMessage& msg);

		std::atomic<bool> mOpen = { false };				///< If this client is currently connected
		nap::Signal<const IWebSocketClient&> destroyed;		///< Called when the client is destroyed
	};


	//////////////////////////////////////////////////////////////////////////
	// WebSocketClient
	//////////////////////////////////////////////////////////////////////////

	class NAPAPI WebSocketClient : public IWebSocketClient
	{
		friend class WebSocketService;
		RTTI_ENABLE(IWebSocketClient)
	public:
		// Constructor used by factory
		WebSocketClient(WebSocketService& service);

		// Destructor
		virtual ~WebSocketClient();

		/**
		 * Initialize this object after de-serialization
		 * @param errorState contains the error message when initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Sends a message with the given opcode to the server.
		 * @param message the message to send
		 * @param code message type
		 * @param error contains the error if sending fails
		 * @return if message was sent successfully
		 */
		bool send(const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error);

		/**
		 * Sends a message using the given payload and opcode to the server.
		 * @param payload the message buffer
		 * @param length total number of bytes
		 * @param code message type
		 * @param error contains the error if sending fails
		 * @return if message was sent successfully
		 */
		bool send(void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error);

		/**
		 * Sends a message to the server.
		 * @param message the message to send
		 * @param error contains the error if sending fails
		 * @return if message was sent successfully.
		 */
		bool send(const WebSocketMessage& message, nap::utility::ErrorState& error);

	protected:
		virtual void onConnectionOpened() override;
		virtual void onConnectionClosed(int code, const std::string& reason) override;
		virtual void onConnectionFailed(int code, const std::string& reason) override;
		virtual void onMessageReceived(const WebSocketMessage& msg) override;

	private:

		/**
		 * Consumes all received web-socket events and moves them to outEvents
		 * Calling this will clear the internal queue and transfers ownership of the events to the caller
		 * @param outEvents will hold the transferred web-socket events
		 */
		void consumeEvents(std::queue<WebSocketEventPtr>& outEvents);

		/**
		 * Called when the end point receives a new event.
		 * Adds the event to the list of events to be processed on the main thread.
		 * @param newEvent the web-socket event.
		 */
		void addEvent(WebSocketEventPtr newEvent);

		WebSocketService* mService = nullptr;

		// Queue that holds all the consumed events
		std::queue<WebSocketEventPtr> mEvents;

		// Mutex associated with setting / getting events
		std::mutex	mEventMutex;
	};

	// Object creator used for constructing the websocket client
	using WebSocketClientObjectCreator = rtti::ObjectCreator<WebSocketClient, WebSocketService>;
}
