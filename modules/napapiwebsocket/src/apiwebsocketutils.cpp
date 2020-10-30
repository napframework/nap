/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "apiwebsocketutils.h"
#include <rtti/typeinfo.h>

RTTI_BEGIN_ENUM(nap::EWebSocketForwardMode)
	RTTI_ENUM_VALUE(nap::EWebSocketForwardMode::WebSocketEvent, "WebSocketEvent"),
	RTTI_ENUM_VALUE(nap::EWebSocketForwardMode::APIEvent, "APIEvent"),
	RTTI_ENUM_VALUE(nap::EWebSocketForwardMode::Both, "Both")
RTTI_END_ENUM

namespace nap
{

}