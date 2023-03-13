/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "websocketevent.h"

RTTI_DEFINE_BASE(nap::WebSocketEvent)
RTTI_DEFINE_BASE(nap::WebSocketConnectionEvent)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WebSocketConnectionClosedEvent)
	RTTI_CONSTRUCTOR(nap::WebSocketConnection, int, const std::string&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WebSocketConnectionOpenedEvent)
	RTTI_CONSTRUCTOR(nap::WebSocketConnection)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WebSocketConnectionFailedEvent)
	RTTI_CONSTRUCTOR(nap::WebSocketConnection, int, const std::string&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WebSocketMessageReceivedEvent)
	RTTI_CONSTRUCTOR(nap::WebSocketConnection, const nap::WebSocketMessage&)
RTTI_END_CLASS

namespace nap
{

}