/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

/// local includes
#include "inputservice.h"
#include "inputevent.h"
#include "inputrouter.h"
#include "window.h"

#include <nap/resource.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::InputService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	InputService::InputService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
		mInputEvents.reserve(5000);
	}

	void InputService::processWindowEvents(Window& window, InputRouter& inputRouter, const EntityList& entities)
	{
		int window_number = static_cast<int>(window.getNumber());

		// Route event
		InputEventPtrList::iterator input_it = mInputEvents.begin();
		while (input_it != mInputEvents.end())
		{
			// Skip events not associated with a specific window
			WindowInputEvent* window_event = rtti_cast<WindowInputEvent>(input_it->get());
			if (window_event == nullptr || window_event->mWindow != window_number)
			{
				++input_it;
				continue;
			}

			// Otherwise perform routing and delete event
			// The iterator points to a unique ptr that needs to be dereferenced
			inputRouter.routeEvent(**input_it, entities);

			// Erase and return next valid iterator
			input_it = mInputEvents.erase(input_it);
		}
	}


	void InputService::processControllerEvents(int deviceID, InputRouter& inputRouter, const EntityList& entities)
	{
		// Route event
		InputEventPtrList::iterator input_it = mInputEvents.begin();
		while (input_it != mInputEvents.end())
		{
			ControllerEvent* controller_event = rtti_cast<ControllerEvent>(input_it->get());
			if (controller_event == nullptr || controller_event->mDeviceID != deviceID)
			{
				++input_it;
				continue;
			}

			// Otherwise perform routing and delete event
			// The iterator points to a unique ptr that needs to be dereferenced
			inputRouter.routeEvent(**input_it, entities);

			// Erase and return next valid iterator
			input_it = mInputEvents.erase(input_it);
		}
	}


	void InputService::processAllEvents(InputRouter& inputRouter, const EntityList& entities)
	{
		// Route window event
		InputEventPtrList::iterator input_it = mInputEvents.begin();
		while (input_it != mInputEvents.end())
		{
			// Otherwise perform routing and delete event
			// The iterator points to a unique ptr that needs to be dereferenced
			inputRouter.routeEvent(**input_it, entities);

			// Erase and return next valid iterator
			input_it = mInputEvents.erase(input_it);
		}
	}


	void InputService::addEvent(InputEventPtr inEvent)
	{
		if (inEvent->get_type() == RTTI_OF(ControllerConnectionEvent))
			controllerConnectionChanged(static_cast<const ControllerConnectionEvent&>(*inEvent));
		mInputEvents.emplace_back(std::move(inEvent));
	}

	
	void InputService::postUpdate(double deltaTime)
	{
		mInputEvents.clear();
	}
}


