#pragma once

#include "rtti/rttiobject.h"

namespace nap
{
	class InputEvent;
	class EntityInstance;

	/**
	 * Base class for input routing. An input router selects InputComponents from a hierarchy of entities
	 * to send the input to.
	 */
	class InputRouter : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)

	public:
		using EntityList = std::vector<EntityInstance*>;


		/**
		 * Processes a single event.
		 * @param event The event to process.
		 * @param entities The list of entities to process recursively.
		 */
		virtual void routeEvent(const InputEvent& event, const EntityList& entities) = 0;
	};

	/**
	 * Default implementation of InputRouter. Sends event to all entities.
	 */
	class DefaultInputRouter : public InputRouter
	{
		RTTI_ENABLE(InputRouter)

	public:
		/**
		 * Sends event to all entities.
		 */
		virtual void routeEvent(const InputEvent& event, const EntityList& entities);
	};
}
