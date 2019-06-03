#pragma once

// Local Includes
#include "websocketconnection.h"

namespace nap
{
	// Forward Declares
	class WebSocketServerEndPoint;

	/**
	 * Utility class that combines a message and connection into a single object.
	 * Can only be constructed by the web socket server endpoint.
	 * This message can be copied and moved freely.
	 */
	class NAPAPI WebSocketMessage final
	{
		friend class WebSocketServerEndPoint;
	public:
		// Default constructor
		WebSocketMessage() = delete;
		// Default move constructor
		WebSocketMessage(WebSocketMessage&& other) = default;
		// Default move assignment operator
		WebSocketMessage& operator=(WebSocketMessage&& other) = default;
		// Default copy constructor
		WebSocketMessage(WebSocketMessage& other) = default;
		// Default copy assignment operator
		WebSocketMessage& operator=(const WebSocketMessage& other) = default;

	private:
		WebSocketMessage(WebSocketConnection connection, wspp::MessagePtr message);
		wspp::MessagePtr mMessage = nullptr;							///< Shared pointer to the message
		WebSocketConnection mConnection;								///< Connection
	};

}
