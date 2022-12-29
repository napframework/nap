/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap Includes
#include <nap/service.h>
#include <utility/dllexport.h>
#include <entity.h>
#include <nap/signalslot.h>

// Local Includes
#include "inputevent.h"

namespace nap
{
	class InputService;
	class Window;
	class EntityInstance;
	class InputRouter;

	/**
	 * Stores and processes input events.
	 * Currently supports two types of input events: ControllerEvent and WindowInputEvent.
	 * Window input events are generated from a window and contain mouse, keyboard and touch events.
	 * Controller events are generated from an external piece of hardware such as a game controller or joystick and are not related to a window
	 * Separate functions are available to forward stored input events to parts of the scene / a set of entities.
	 * It's important to call process..() at least once every frame! 
	 * Dangling events, ie: events that are not processed, are cleared automatically each frame.
	 */
	class NAPAPI InputService : public Service
	{
		RTTI_ENABLE(Service)

	public:
		// Default constructor
		InputService(ServiceConfiguration* configuration) ;

		// Disable copy
		InputService(const InputService& that) = delete;
		InputService& operator=(const InputService&) = delete;
		
		/**
		 * Forwards all input events, generated from a window, to all input components that are part of the list of entities
		 * The input router controls how the events are forwarded. A default input router will suffice in most cases
		 * The need for this as a separate function is because the same parts of a scene can be drawn to different windows.
		 * By explicitly giving a window the user can associate events coming from a specific window to a specific part of the scene.
		 * @param window The window to process the input events for (mouse, keyboard touch etc.)
		 * @param inputRouter The input router that selects what InputComponents receive input messages.
		 * @param entities A list of root entities that are used to traverse the entity hierarchy.
		 */
		void processWindowEvents(Window& window, InputRouter& inputRouter, const EntityList& entities);

		/**
		* Forwards all input events associated with a specific 'game' controller, to all input components that are part of the list of entities
		* The input router controls how the events are forwarded. A default input router will suffice in most cases
		* @param controllerID the game controller id, starting from 0
		* @param inputRouter The input router that selects what InputComponents receive input messages.
		* @param entities A list of root entities that are used to traverse the entity hierarchy.
		*/
		void processControllerEvents(int controllerID, InputRouter& inputRouter, const EntityList& entities);

		/**
		 * Utility function that processes all input events.
		 * This call forwards all events, regardless of where they come from, to the given list of entities.
		 * This only works when there is exactly 1 window and therefore no reason to distinguish between windows.
		 * When there are multiple windows call processWindowEvents and processControllerEvents individually!
		 * Individual calls ensure that input events from a certain window are forwarded to the right part of the system.
		 * @param inputRouter The input router that selects what InputComponents receive input messages.
		 * @param entities A list of root entities that are used to traverse the entity hierarchy.
		 */
		void processAllEvents(InputRouter& inputRouter, const EntityList& entities);

		/**
		 * Adds an input event to the queue, to be processed later.
		 * @param inEvent the event to add, ownership is transfered here
		 */
		void addEvent(InputEventPtr inEvent);

		/**
		 * Clears all dangling input events.
		 */
		virtual void postUpdate(double deltaTime);

		Signal<const ControllerConnectionEvent&>	controllerConnectionChanged;		///< Signal emitted when a controller is removed or added

	private:
		InputEventPtrList mInputEvents;			///< All input events
	};
}
