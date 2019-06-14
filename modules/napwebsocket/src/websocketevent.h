#pragma once

// Local Includes
#include "websocketmessage.h"
#include "websocketconnection.h"

// External Includes
#include <nap/event.h>

namespace nap
{
	/**
	 * Base class of all web socket related events
	 */
	class NAPAPI WebSocketEvent : public Event
	{
		RTTI_ENABLE(Event)
	};


	/**
	 *  Base class of all web-socket connection related events.
	 */
	class NAPAPI WebSocketConnectionEvent : public WebSocketEvent
	{
		RTTI_ENABLE(WebSocketEvent)
	public:
		WebSocketConnectionEvent(const WebSocketConnection& connection) :
			mConnection(connection) { }

		WebSocketConnection mConnection;
	};


	/**
	 * Occurs when a connection is closed. Contains the reason and exit code.
	 * Note that the connection itself is invalided when this event is received.
	 */
	class NAPAPI WebSocketConnectionClosedEvent : public WebSocketConnectionEvent
	{
		RTTI_ENABLE(WebSocketConnectionEvent)
	public:
		WebSocketConnectionClosedEvent(const WebSocketConnection& connection, int errorCode, const std::string& reason) :
			WebSocketConnectionEvent(connection), mErrorCode(errorCode), mReason(reason)	{ }

		WebSocketConnectionClosedEvent(const WebSocketConnection& connection, int errorCode, std::string&& reason) :
			WebSocketConnectionEvent(connection), mErrorCode(errorCode), mReason(std::move(reason)) { }

		int mErrorCode = -1;		///< Error code associated with reason for closing connection
		std::string mReason;		///< Reason for closing connection
	};


	/**
	 * Occurs when a new connection to a client or server is opened.
	 * The connection will be valid when this event is received.
	 */
	class NAPAPI WebSocketConnectionOpenedEvent : public WebSocketConnectionEvent
	{
		RTTI_ENABLE(WebSocketConnectionEvent)
	public:
		WebSocketConnectionOpenedEvent(const WebSocketConnection& connection) :
			WebSocketConnectionEvent(connection) { }
	};


	/**
	 * Occurs when a connection failed to open. Contains the error code and reason for failure.
	 * Note that the connection itself is invalided when this event is received.
	 */
	class NAPAPI WebSocketConnectionFailedEvent : public WebSocketConnectionEvent
	{
		RTTI_ENABLE(WebSocketConnectionEvent)
	public:
		WebSocketConnectionFailedEvent(const WebSocketConnection& connection, int errorCode, const std::string& reason) :
			WebSocketConnectionEvent(connection), mErrorCode(errorCode), mReason(reason)	{ }

		WebSocketConnectionFailedEvent(const WebSocketConnection& connection, int errorCode, std::string&& reason) :
			WebSocketConnectionEvent(connection), mErrorCode(errorCode), mReason(reason) { }

		int mErrorCode = -1;	///< Error code associated with reason for failing to establish connection
		std::string mReason;	///< Reason for failing to establish connection
	};


	/**
	 * Occurs when a message is received. 
	 * Contains the message and handle to the connection that made the request.
	 * The connection will be valid when this event is received.
	 */
	class NAPAPI WebSocketMessageReceivedEvent : public WebSocketEvent
	{
		RTTI_ENABLE(WebSocketEvent)
	public:
		WebSocketMessageReceivedEvent(const WebSocketConnection& connection, const WebSocketMessage& message) : 
			mConnection(connection), mMessage(message)	{ }

		WebSocketMessageReceivedEvent(const WebSocketConnection& connection, WebSocketMessage&& message) :
			mConnection(connection),
			mMessage(std::move(message))	{ }

        WebSocketConnection mConnection;    ///< Handle to the client or server connection.
		WebSocketMessage mMessage;			///< Message received from a client or server.
	};

	using WebSocketEventPtr = std::unique_ptr<nap::WebSocketEvent>;
}
