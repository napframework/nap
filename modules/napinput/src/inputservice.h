#pragma once

// Nap Includes
#include <nap/service.h>
#include <utility/dllexport.h>
#include <entity.h>

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
		 * Forwards all input events, associated with a window, to all input components that are part of the list of entities
		 * The input router controls how the events are forwarded. A default input router will suffice in most cases
		 * @param window The window to process the input events for (mouse, keyboard touch etc.)
		 * @param inputRouter The input router that selects what InputComponents receive input messages.
		 * @param entities A list of root entities that are used to traverse the entity hierarchy.
		 */
		void processEvents(Window& window, InputRouter& inputRouter, const EntityList& entities);

		/**
		 * Adds an input event to the queue, to be processed later
		 * @param inputEvent, the event to add, ownership is transfered here
		 */
		void addEvent(InputEventPtr inEvent);

	private:
		InputEventPtrList mInputEvents;
	};
}