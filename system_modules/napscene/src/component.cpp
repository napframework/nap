/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "component.h"
#include "entity.h"
#include <nap/python.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Component, "Behavioural trait (charactaristic, quality) of an entity")
RTTI_END_CLASS

namespace nap
{
    bool ComponentInstance::init(utility::ErrorState& errorState)
    {
        return true;
    }


	void ComponentInstance::addToComponentLinkMap(Component* targetResource, const std::string& instancePath, ComponentInstance** targetInstancePtr)
	{
		mComponentLinkMap[targetResource].push_back({ targetInstancePtr, instancePath } );
	}


	void ComponentInstance::addToEntityLinkMap(Entity* targetResource, const std::string& instancePath, EntityInstance** targetInstancePtr)
	{
		mEntityLinkMap[targetResource].push_back({ targetInstancePtr, instancePath });
	}


	const std::string& Component::getOriginalID() const
	{
		return mOriginalComponent != nullptr ? mOriginalComponent->mID : mID;
	}
}
