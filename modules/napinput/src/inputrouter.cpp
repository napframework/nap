// local includes
#include "inputevent.h"
#include "inputcomponent.h"

// External includes
#include <inputrouter.h>
#include <entity.h>

RTTI_BEGIN_CLASS(nap::DefaultInputRouterComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::DefaultInputRouterComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{

	void DefaultInputRouter::routeEvent(const InputEvent& event, const EntityList& entities)
	{
		for (EntityInstance* entity : entities)
		{
			std::vector<InputComponentInstance*> input_components;
			if (mRecursive)
			{
				entity->getComponentsOfTypeRecursive<InputComponentInstance>(input_components);
			}
			else
			{
				entity->getComponentsOfType<InputComponentInstance>(input_components);
			}

			// Trigger all found components
			for (InputComponentInstance* component : input_components)
			{
				component->trigger(event);
			}
		}
	}
}


