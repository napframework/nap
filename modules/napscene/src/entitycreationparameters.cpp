/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "entitycreationparameters.h"
#include "entity.h"

namespace nap
{
	ClonedComponentResource::ClonedComponentResource(const ComponentResourcePath& path, std::unique_ptr<Component> resource) :
		mPath(path),
		mResource(std::move(resource))
	{
	}

	EntityCreationParameters::EntityCreationParameters(const EntityObjectGraph& objectGraph) :
		mObjectGraph(&objectGraph)
	{
	}

	EntityCreationParameters::~EntityCreationParameters()
	{
	}
}