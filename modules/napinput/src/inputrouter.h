/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <rtti/object.h>
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
		 *	Default constructor
		 */
		DefaultInputRouter() = default;

		/**
		 * Constructor that sets the recursive flag
		 * @param recursive if child entities are taken into consideration
		 */
		DefaultInputRouter(bool recursive) : mRecursive(recursive)			{ }

		/**
		 * Sends event to all entities in the entity list.
		 * Note that when recursive is set to true child entities are considered as well
		 * @param event the event to forward to the list of entities
		 * @param entities the entities to forward the events to
		 */
		virtual void routeEvent(const InputEvent& event, const EntityList& entities);

		/**
		 * @param value If input events are forward recursively to child entities
		 */
		void setRecursive(bool value)			{ mRecursive = value; }

		/**
		 * @return If input events are forwarded recursively to child entities
		 */
		bool isRecursive() const				{ return mRecursive; }

	private:
		bool mRecursive = false;
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
