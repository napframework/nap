/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

namespace nap
{
	/**
	 * Enum that describes how a nap::APIWebSocketClient and nap::APIWebSocketServer forward web-socket messages.
	 */
	enum class EWebSocketForwardMode : int
	{
		WebSocketEvent	= 0,		///< Create and forward web socket events only	
		APIEvent		= 1,		///< Create and forward api events only
		Both			= 2			///< Create and forward both web-socket and api events
	};
}