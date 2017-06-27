/// local includes
#include <inputservice.h>
#include "inputevent.h"
#include "inputrouter.h"
#include "nap/windowresource.h"

RTTI_BEGIN_CLASS(nap::InputService)
RTTI_END_CLASS

namespace nap
{
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


