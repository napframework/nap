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
	 *  Base class for all web-socket connection related events
	 */
	class NAPAPI WebSocketConnectionEvent : public WebSocketEvent
	{
		RTTI_ENABLE(WebSocketEvent)
	public:
		WebSocketConnectionEvent(WebSocketConnection connection) :
			mConnection(connection) { }
	private:
		WebSocketConnection mConnection;
	};


	/**
	 * Occurs when a connection is closed
	 */
	class NAPAPI WebSocketConnectionClosedEvent : public WebSocketConnectionEvent
	{
		RTTI_ENABLE(WebSocketConnectionEvent)
	public:
		WebSocketConnectionClosedEvent(WebSocketConnection connection, int errorCode, const std::string& reason) :
			WebSocketConnectionEvent(connection), mErrorCode(errorCode), mReason(reason)	{ }

		int mErrorCode = -1;		///< Error code associated with reason for closing connection
		std::string mReason;		///< Reason for closing connection
	};


	/**
	 * Occurs when a connection is opened
	 */
	class NAPAPI WebSocketConnectionOpenedEvent : public WebSocketConnectionEvent
	{
		RTTI_ENABLE(WebSocketConnectionEvent)
	public:
		WebSocketConnectionOpenedEvent(WebSocketConnection connection) :
			WebSocketConnectionEvent(connection) { }
	};


	/**
	 * Occurs when a connection failed to open
	 */
	class NAPAPI WebSocketConnectionFailedEvent : public WebSocketConnectionEvent
	{
		RTTI_ENABLE(WebSocketConnectionEvent)
	public:
		WebSocketConnectionFailedEvent(WebSocketConnection connection, int errorCode, const std::string& reason) :
			WebSocketConnectionEvent(connection), mErrorCode(errorCode), mReason(reason)	{ }

		int mErrorCode = -1;	///< Error code associated with reason for failing to establish connection
		std::string mReason;	///< Reason for failing to establish connection
	};


	/**
	 * A received web-socket message
	 */
	class NAPAPI WebSocketMessageReceivedEvent : public WebSocketEvent
	{
		RTTI_ENABLE(WebSocketEvent)
	public:
		WebSocketMessageReceivedEvent(WebSocketConnection connection, WebSocketMessage message) : 
			mConnection(connection), mMessage(message)	{ }
	private:
		WebSocketMessage mMessage;
		WebSocketConnection mConnection;
	};

	using WebSocketEventPtr = std::unique_ptr<nap::WebSocketEvent>;
}
