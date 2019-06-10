#pragma once

// External Includes
#include <apievent.h>
#include <websocketconnection.h>
#include <websocketinterface.h>

namespace nap
{
	/**
	 * Input / Output message associated with a specific web-socket (client / server) connection. 
	 * This class adds web-socket functionality to a default APIEvent.
	 * To dispatch this event to an external environment call APIService::dispatchEvent after construction of this event.
	 * The APIWebSocketDispatcher dispatches this type of event to an external environment using it's linked endpoint.
	 * It is important that the connection (stored in this object) is managed by the endpoint linked to by the dispatcher.
	 */
	class NAPAPI APIWebSocketEvent : public APIEvent
	{
		RTTI_ENABLE(APIEvent)
	public:
		/**
		 * Constructs a new web socket event with the given name and web-socket connection.
		 * A unique id is generated for this event.
		 * @param name the name of this call
		 * @param connection the web-socket endpoint connection.
		 */
		APIWebSocketEvent(const std::string& name, const WebSocketConnection& connection, 
			WebSocketInterface& wsInterface);

		/**
		 * Constructs a new web socket event with the given name and web-socket connection.
		 * A unique id is generated for this event.
		 * @param name the name of this call
		 * @param connection the web-socket endpoint connection.
		 */
		APIWebSocketEvent(std::string&& name, const WebSocketConnection& connection, 
			WebSocketInterface& wsInterface);

		/**
		 * Constructs a new web socket event with the given name, unique id and web-socket connection.
		 * @param name the name of this call
		 * @param id unique identifier of this call
		 * @param connection the web-socket endpoint connection.
		 */
		APIWebSocketEvent(const std::string& name, const std::string& id, const WebSocketConnection& connection, 
			WebSocketInterface& wsInterface);

		/**
		 * Constructs a new web socket event with the given name, unique id and web-socket connection.
		 * @param name the name of this call
		 * @param id unique identifier of this call
		 * @param connection the web-socket endpoint connection.
		 */
		APIWebSocketEvent(std::string&& name, std::string&& id, const WebSocketConnection& connection, 
			WebSocketInterface& wsInterface);

		/**
		 * @return The web-socket connection associated with this event
		 */
		const WebSocketConnection& getConnection() const			{ return mConnection; }

		/**
		 * Returns the interface that created this web-socket event.
		 * The interface is either an api server or client of type: APIWebSocketClient or APIWebSocketServer.
		 * @return The web-socket interface associated with this event.
		 */
		WebSocketInterface& getInterface();

		/**
		 * Returns the interface that created this web-socket event.
		 * The interface is either an api server or client of type: APIWebSocketClient or APIWebSocketServer.
		 * @return The web-socket interface associated with this event.
		 */
		const WebSocketInterface& getInterface() const;

	private:
		WebSocketConnection mConnection;
		WebSocketInterface* mInterface = nullptr;
	};

	using APIWebSocketEventPtr = std::unique_ptr<APIWebSocketEvent>;
}
