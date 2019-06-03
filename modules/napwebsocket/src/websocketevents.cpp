#include "websocketevents.h"

RTTI_DEFINE_BASE(nap::WebSocketEvent)
RTTI_DEFINE_BASE(nap::WebSocketConnectionEvent)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WebSocketConnectionClosedEvent)
	RTTI_CONSTRUCTOR(nap::WebSocketConnection)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WebSocketConnectionOpenedEvent)
	RTTI_CONSTRUCTOR(nap::WebSocketConnection)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WebSocketConnectionFailedEvent)
	RTTI_CONSTRUCTOR(nap::WebSocketConnection)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WebSocketMessageReceivedEvent)
	RTTI_CONSTRUCTOR(nap::WebSocketMessage)
RTTI_END_CLASS

namespace nap
{

}