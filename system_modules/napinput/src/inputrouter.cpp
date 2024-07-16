/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "inputevent.h"
#include "inputcomponent.h"

// External includes
#include <inputrouter.h>
#include <entity.h>

RTTI_BEGIN_CLASS(nap::DefaultInputRouterComponent, "Creates a default input router, used to forward input events to a selection of entities")
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
			// Get input components
			std::vector<InputComponentInstance*> input_components;
			mRecursive ? entity->getComponentsOfTypeRecursive<InputComponentInstance>(input_components) :
				entity->getComponentsOfType<InputComponentInstance>(input_components);

			// Trigger all found components
			for (InputComponentInstance* component : input_components)
				component->trigger(event);
		}
	}
}


