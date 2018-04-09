/// local includes
#include "inputservice.h"
#include "inputevent.h"
#include "inputrouter.h"

#include <nap/windowresource.h>
#include <nap/resource.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::InputService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	InputService::InputService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}

	void InputService::processEvents(Window& window, InputRouter& inputRouter, const EntityList& entities)
	{
		int window_number = static_cast<int>(window.getNumber());

		// Route event
		InputEventPtrList::iterator input_it = mInputEvents.begin();
		while (input_it != mInputEvents.end())
		{
			// Skip events not associated with a specific window
			if ((*input_it)->mWindow != window_number)
			{
				++input_it;
				continue;
			}

			// Otherwise perform routing and delete event
			// The iterator points to a unique ptr that needs to be dereferenced
			inputRouter.routeEvent(**input_it, entities);

			// Erase and return next valid iterator
			input_it = mInputEvents.erase(input_it);
		}
	}


	void InputService::addEvent(InputEventPtr inEvent)
	{
		mInputEvents.emplace_back(std::move(inEvent));
	}

}


