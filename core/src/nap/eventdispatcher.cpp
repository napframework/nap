// Local Includes
#include "eventdispatcher.h"

#include <algorithm>

namespace nap
{
	// Maps type to string
	const DispatchMethodMap gDispatchMethods =
	{
		{ DispatchMethod::Upstream,   "UpStream" },
		{ DispatchMethod::Siblings,   "Siblings" },
		{ DispatchMethod::Downstream, "Downstream" }
	};


	// Converts string to dispatch method
	bool convert_string_to_dispatchmethod(const std::string& inValue, DispatchMethod& outMethod)
	{
		// Find the right blend mode
		auto it = std::find_if(gDispatchMethods.begin(), gDispatchMethods.end(), [&](const auto& vt) {
			return vt.second == inValue; });

		if (it == gDispatchMethods.end())
		{
			Logger::warn("Unable to find dispatch mode with name: " + inValue);
			return false;
		}

		// Set out value
		outMethod = it->first;
		return true;
	}


	// Converts dispatch method to string
	bool convert_dispatchmethod_to_string(const DispatchMethod& inMethod, std::string& outString)
	{
		DispatchMethod mode = inMethod;
		const auto& it = gDispatchMethods.find(inMethod);
		if (it == gDispatchMethods.end())
		{
			Logger::warn("Unable to find dispatch mode in map");
			return false;
		}
		outString = it->second;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////


	// Dispatches events to handlers directly installed on the entity, returns true when the event has been handled
	bool dispatchEventToSiblings(Entity& entity, Event& evt)
	{
		for (EventHandlerComponent* handler : entity.getComponentsOfType<EventHandlerComponent>())
		{
			if (handler->handleEvent(evt))
			{
				evt.setHandled(true);
				return true;
			}
		}
		return false;
	}


	// Recursive function that sends an event up stream, returns true when the event has been handled
	bool dispatchEventUpStream(Entity& entity, Event& evt)
	{
		Entity* parentEntity = entity.getParent();
		if (!parentEntity)
			return false;

		for (auto& handler : parentEntity->getComponentsOfType<EventHandlerComponent>())
		{
			if (handler->handleEvent(evt))
			{
				evt.setHandled(true);
				return true;
			}
		}

		return dispatchEventUpStream(*parentEntity, evt);
	}


	// Recursive function that dispatches components down the tree, returns true when the event has been handled
	bool dispatchEventDownstream(Entity& entity, Event& evt)
	{
		for (auto& childEntity : entity.getEntities()) 
		{
			for (auto& handler : childEntity->getComponentsOfType<EventHandlerComponent>()) 
			{
				if (handler->handleEvent(evt))
				{
					evt.setHandled(true);
					return true;
				}
			}
			if (dispatchEventDownstream(*childEntity, evt))
				return true;
		}
		return false;
	}


	EventDispatcher::EventDispatcher(Object* parent, const std::string& name)
	{
		setParent(*parent);
		setName(name);
	}


	// Dispatches the @inEvent based on the direction attribute
	void EventDispatcher::dispatchEvent(Event& event, Object& source)
	{
		Entity* entity = nullptr;
        if (source.getTypeInfo().isKindOf<Entity>()) {
            entity = static_cast<Entity*>(&source);
        } else if (source.getTypeInfo().isKindOf<Component>()) {
            entity = static_cast<Component*>(&source)->getParent();
        } else {
            Logger::fatal("Wrong type of source object");
        }

        event.setSource(source);

		assert(entity);

		switch (mDirection)
		{
		case DispatchMethod::Upstream:
			dispatchEventUpStream(*entity, event);
			break;
		case DispatchMethod::Siblings:
			dispatchEventToSiblings(*entity, event);
			break;
		case DispatchMethod::Downstream:
			dispatchEventDownstream(*entity, event);
			break;
		}
	}
}

RTTI_DEFINE(nap::EventDispatcher)
RTTI_DEFINE_DATA(nap::DispatchMethod)
