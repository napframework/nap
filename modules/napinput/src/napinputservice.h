#pragma once

// Nap Includes
#include <nap/service.h>

namespace nap
{
	class InputService;
	class WindowResource;
	class EntityInstance;
	class InputRouter;

	/**
	 * Extracts input events from Window and forwards them to an InputRouter.
	 */
	class InputService : public Service
	{
		RTTI_ENABLE(Service)

	public:
		using EntityList = std::vector<EntityInstance*>;

		// Default constructor
		InputService() = default;

		// Disable copy
		InputService(const InputService& that) = delete;
		InputService& operator=(const InputService&) = delete;
		
		/**
		 * Extracts input events from Window and forwards them to an InputRouter.
		 * @param window The window to extract events from. Note: Window::processEvents must be called after this function, as it clears the event queue.
		 * @param inputRouter The input router that selects what InputComponents receive input messages.
		 * @param entities A list of root entities that are used to traverse the entity hierarchy.
		 */
		void handleInput(WindowResource& window, InputRouter& inputRouter, const EntityList& entities);
	};
}