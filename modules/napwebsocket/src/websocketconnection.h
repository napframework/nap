#pragma once

// Local Includes
#include "wspp.h"

// External Includes
#include <utility/dllexport.h>

namespace nap
{
	class WebSocketServerEndPoint;

	/**
	 * Utility class that wraps a web-socket connection handle.
	 * Can only be constructed web socket server end points.
	 * The object is light and can be copied and or moved around freely.
	 */
	class NAPAPI WebSocketConnection final
	{
		friend class WebSocketServerEndPoint;

	public:
		// Default constructor
		WebSocketConnection() = delete;
		// Default move constructor
		WebSocketConnection(WebSocketConnection&& other) = default;
		// Default move assignment operator
		WebSocketConnection& operator=(WebSocketConnection&& other) = default;
		// Default copy constructor
		WebSocketConnection(const WebSocketConnection& other) = default;
		// Default copy assignment operator
		WebSocketConnection& operator=(const WebSocketConnection& other) = default;

	private:
		/**
		 * Constructs a web-socket connection based on the underling websocketpp connection handle
		 * @param connection the websocketpp connection handle
		 */
		WebSocketConnection(wspp::ConnectionHandle connection);

		wspp::ConnectionHandle mConnection;		///< Weak pointer to the underlying websocketpp connection
	};
}
