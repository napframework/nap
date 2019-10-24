#pragma once

// External Includes
#include <apievent.h>
#include <websocketconnection.h>
#include <websocketinterface.h>

namespace nap
{
	/*
	 * Adds web-socket functionality to a default nap::APIEvent. Use getConnection() 
	 * to access the webs-socket connection handle to the client or server. Note 
	 * that the handle could be invalid when a client or server disconnected or the connection
	 * failed to establish. Use getInterface() to access the client or server associated with event. Most
	 * likely the interface is a nap::APIWebSocketClient or nap::APIWebSocketServer.
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
		 * @param wsInterface interface this event originated from (client or server)
		 */
		APIWebSocketEvent(const std::string& name, const WebSocketConnection& connection, 
			WebSocketInterface& wsInterface);

		/**
		 * Constructs a new web socket event with the given name and web-socket connection.
		 * A unique id is generated for this event.
		 * @param name the name of this call
		 * @param connection the web-socket endpoint connection.
		 * @param wsInterface interface this event originated from (client or server)
		 */
		APIWebSocketEvent(std::string&& name, const WebSocketConnection& connection, 
			WebSocketInterface& wsInterface);

		/**
		 * Constructs a new web socket event with the given name, unique id and web-socket connection.
		 * Use this constructor to form a reply based on a previously received client request. 
		 * The uuid should match the uuid of the request. This allows the client to match call id's.
		 * @param name the name of this call
		 * @param id unique identifier of this call
		 * @param connection the web-socket endpoint connection.
		 * @param wsInterface interface this event originated from (client or server)
		 */
		APIWebSocketEvent(const std::string& name, const std::string& id, const WebSocketConnection& connection, 
			WebSocketInterface& wsInterface);

		/**
		 * Constructs a new web socket event with the given name, unique id and web-socket connection.
		 * Use this constructor to form a reply based on a previously received client request. 
		 * The uuid should match the uuid of the request. This allows the client to match call id's.
		 * @param name the name of this call
		 * @param id unique identifier of this call
		 * @param connection the web-socket endpoint connection.
		 * @param wsInterface interface this event originated from (client or server)
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
