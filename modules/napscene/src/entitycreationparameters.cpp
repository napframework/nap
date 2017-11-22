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