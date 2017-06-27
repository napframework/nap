// local includes
#include <inputrouter.h>
#include "inputevent.h"
#include "inputcomponent.h"

RTTI_BEGIN_BASE_CLASS(nap::InputRouter)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::DefaultInputRouter)
RTTI_END_CLASS


namespace nap
{

	void DefaultInputRouter::routeEvent(const InputEvent& event, const EntityList& entities)
	{
		for (EntityInstance* entity : entities)
		{
			std::vector<InputComponent*> input_components;
			entity->getComponentsOfType<InputComponent>(input_components);

			for (InputComponent* component : input_components)
			{
				component->trigger(event);
			}
		}
	}

}


