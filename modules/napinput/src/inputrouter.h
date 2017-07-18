#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <nap/componentinstance.h>
#include <nap/dllexport.h>

namespace nap
{
	class InputEvent;
	class EntityInstance;
	class DefaultInputRouterComponent;

	/**
	 * Base class for input routing. An input router selects InputComponents from a hierarchy of entities
	 * to send the input to.
	 */
	class NAPAPI InputRouter
	{
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
	class NAPAPI DefaultInputRouter : public InputRouter
	{
	public:
		/**
		 * Sends event to all entities.
		 */
		virtual void routeEvent(const InputEvent& event, const EntityList& entities);
	};


	/**
	 * Component used to have a default input router entity
	 */
	class NAPAPI DefaultInputRouterComponentResource : public ComponentResource
	{
		RTTI_ENABLE(ComponentResource)

	public:
		/**
		 * @return Instance type to create for this resource.
		 */
		virtual const rtti::TypeInfo getInstanceType() const override	{ return RTTI_OF(DefaultInputRouterComponent); }
	};


	/**
	 * Wrapper component for Default Input Router
	 */
	class NAPAPI DefaultInputRouterComponent : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)

	public:
		DefaultInputRouterComponent(EntityInstance& entity) :
			ComponentInstance(entity)
		{
		}

		DefaultInputRouter mInputRouter;		// Default input router
	};
}
