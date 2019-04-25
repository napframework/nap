#include <nap/python.h>
#include "component.h"
#include "entity.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Component)
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
		if (mOriginalComponent != nullptr)
			return mOriginalComponent->mID;

		return mID;
	}
}
