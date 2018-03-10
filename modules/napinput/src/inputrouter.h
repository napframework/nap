#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <component.h>
#include <utility/dllexport.h>

namespace nap
{
	class InputEvent;
	class EntityInstance;
	class DefaultInputRouterComponentInstance;

	/**
	 * Base class for routing input events. An input router selects input components from a hierarchy of entities.
	 * This class provides an interface to forward input events to input components that belong to one of the entities.
	 * Override the routing call to implement custom routing behavior. 
	 */
	class NAPAPI InputRouter
	{
	public:
		using EntityList = std::vector<EntityInstance*>;

		/**
		 * Forwards an input event to a set of entities.
		 * @param event The event to process.
		 * @param entities The list of entities to process recursively.
		 */
		virtual void routeEvent(const InputEvent& event, const EntityList& entities) = 0;
	};


	/**
	 * Default implementation of an input router. 
	 * Sends input events to all input components that are associated with the selection of entities.
	 */
	class NAPAPI DefaultInputRouter : public InputRouter
	{
	public:
		/**
		 * Sends event to all entities.
		 */
		virtual void routeEvent(const InputEvent& event, const EntityList& entities);
	};


	/**
	 * Allows you to define a default input router as a component in json
	 * When instantiated this component holds a default input router that can be
	 * used to forward events to a selection of entities
	 */
	class NAPAPI DefaultInputRouterComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(DefaultInputRouterComponent, DefaultInputRouterComponentInstance)
	};


	/**
	 * Instance part of the default input router component
	 * Use the input router to forward input events to a selection of entities
	 */
	class NAPAPI DefaultInputRouterComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)

	public:
		DefaultInputRouterComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)
		{
		}

		DefaultInputRouter mInputRouter;		// Default input router
	};
}
