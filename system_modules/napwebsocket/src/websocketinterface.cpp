/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "websocketinterface.h"
#include "websocketservice.h"

// nap::websocketinterface run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WebSocketInterface, "Websocket client or server")
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{

	WebSocketInterface::WebSocketInterface(WebSocketService& service) : mService(&service)
	{

	}


	WebSocketInterface::~WebSocketInterface() 
	{
		if (mRegistered)
		{
			mService->removeInterface(*this);
			mRegistered = false;
		}
	}


	bool WebSocketInterface::init(utility::ErrorState& errorState)
	{
		mService->registerInterface(*this);
		mRegistered = true;
		return true;
	}


	void WebSocketInterface::addEvent(WebSocketEventPtr newEvent)
	{
		std::lock_guard<std::mutex> lock(mEventMutex);
		mEvents.emplace(std::move(newEvent));
	}


	void WebSocketInterface::consumeEvents(std::queue<WebSocketEventPtr>& outEvents)
	{
		// Swap events
		std::lock_guard<std::mutex> lock(mEventMutex);
		outEvents.swap(mEvents);

		// Clear current queue
		std::queue<WebSocketEventPtr> empty_queue;;
		mEvents.swap(empty_queue);
	}

}
