#pragma once

// Local Includes
#include "wspp.h"

// External Includes
#include <utility/dllexport.h>

namespace nap
{
	class WebSocketServerEndPoint;
	class WebSocketClientEndPoint;

	/**
	 * Utility class that wraps a web-socket connection handle. By default the connection is invalid.
	 * Only web-socket endpoints can construct a valid web-socket connection.
	 * The object is light and can be copied and or moved around freely.
	 */
	class NAPAPI WebSocketConnection final
	{
		friend class WebSocketServerEndPoint;
		friend class WebSocketClientEndPoint;

	public:
		// Creates an invalid (expired) connection
		WebSocketConnection() = default;
		// Default move constructor
		WebSocketConnection(WebSocketConnection&& other) = default;
		// Default move assignment operator
		WebSocketConnection& operator=(WebSocketConnection&& other) = default;
		// Default copy constructor
		WebSocketConnection(const WebSocketConnection& other) = default;
		// Default copy assignment operator
		WebSocketConnection& operator=(const WebSocketConnection& other) = default;

		/**
		 * An expired connection isn't managed by an endpoint.
		 * Not to be confused by the connection status.
		 * @return if this connection expired.
		 */
		bool expired() const;

	private:
		/**
		 * Constructs a web-socket connection based on the underling websocketpp connection handle
		 * @param connection the websocketpp connection handle
		 */
		WebSocketConnection(wspp::ConnectionHandle connection);

		wspp::ConnectionHandle mConnection;		///< Weak pointer to the underlying websocketpp connection, invalid by default.
	};
}
