/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
	 * Utility class that wraps a web-socket connection handle.
	 * Only web-socket endpoints can construct a valid nap::WebSocketConnection.
	 * Use this object in combination with an interface (client or server) to send messages.
	 * By default the connection is invalid.
	 * The object is light and can be copied and moved around freely.
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

	private:
		/**
		 * Constructs a web-socket connection based on the underling websocketpp connection handle
		 * @param connection the websocketpp connection handle
		 */
		WebSocketConnection(wspp::ConnectionHandle connection);

		wspp::ConnectionHandle mConnection;		///< Weak pointer to the underlying websocketpp connection, invalid by default.
	};

	using WebSocketConnectionPtr = std::unique_ptr<WebSocketConnection>;
}
