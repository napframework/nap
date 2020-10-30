/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "apiwebsocketevent.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APIWebSocketEvent)
	RTTI_CONSTRUCTOR(const std::string&, const nap::WebSocketConnection&, nap::WebSocketInterface&)
	RTTI_CONSTRUCTOR(const std::string&, const std::string&, const nap::WebSocketConnection&, nap::WebSocketInterface&)
RTTI_END_CLASS

namespace nap
{
	APIWebSocketEvent::APIWebSocketEvent(const std::string& name, const WebSocketConnection& connection, WebSocketInterface& wsInterface) :
		APIEvent(name), mConnection(connection), mInterface(&wsInterface)
	{

	}

	APIWebSocketEvent::APIWebSocketEvent(std::string&& name, const WebSocketConnection& connection, WebSocketInterface& wsInterface) :
		APIEvent(std::move(name)), mConnection(connection), mInterface(&wsInterface)
	{

	}


	APIWebSocketEvent::APIWebSocketEvent(const std::string& name, const std::string& id, const WebSocketConnection& connection, WebSocketInterface& wsInterface) :
		APIEvent(name, id), mConnection(connection), mInterface(&wsInterface)
	{

	}


	APIWebSocketEvent::APIWebSocketEvent(std::string&& name, std::string&& id, const WebSocketConnection& connection, WebSocketInterface& wsInterface) :
		APIEvent(std::move(name), std::move(id)), mConnection(connection), mInterface(&wsInterface)
	{

	}


	nap::WebSocketInterface& APIWebSocketEvent::getInterface()
	{
		assert(mInterface != nullptr);
		return *mInterface;
	}


	const nap::WebSocketInterface& APIWebSocketEvent::getInterface() const
	{
		assert(mInterface != nullptr);
		return *mInterface;
	}

}