#include "entitycreationparameters.h"
#include "entity.h"

namespace nap
{
	ClonedComponentResource::ClonedComponentResource(const std::string& path, std::unique_ptr<Component> resource) :
		mPath(path),
		mResource(std::move(resource))
	{
	}

	EntityCreationParameters::EntityCreationParameters(const RTTIObjectGraph& objectGraph) :
		mObjectGraph(&objectGraph)
	{
	}

	EntityCreationParameters::~EntityCreationParameters()
	{
	}
}