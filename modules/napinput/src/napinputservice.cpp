// Nap includes
#include <nap/core.h>

// local includes
#include <napinputservice.h>
#include "nap/windowresource.h"
#include "napinputevent.h"
#include "napinputcomponent.h"

RTTI_BEGIN_CLASS(nap::InputService)
RTTI_END_CLASS

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

	void InputService::handleInput(WindowResource& window, InputRouter& inputRouter, const EntityList& entities)
	{
		for (const Event* event : window.GetEvents())
		{
			const InputEvent* input_event = rtti_cast<const InputEvent>(event);
			if (input_event == nullptr)
				continue;

			inputRouter.routeEvent(*input_event, entities);
		}
	}
}


