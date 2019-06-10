#pragma once

namespace nap
{
	/**
	 * Enum that describes how an api client and api server forward web-socket messages.
	 */
	enum class EWebSocketForwardMode : int
	{
		WebSocketEvent	= 0,		///< Forward as web socket event only	
		APIEvent		= 1,		///< Forward as api event only
		Both			= 2			///< Forward both as web-socket and api event
	};
}
