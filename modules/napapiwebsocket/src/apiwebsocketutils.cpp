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