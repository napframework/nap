#include <rtti/pythonmodule.h>
#include "nap/component.h"
#include "nap/entity.h"

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

	// Adds component pointer to the internal link map, that is used by the resource manager later to resolve pointers
	// @param targetResource:		component that is being pointed to.
	// @param targetInstancePtr:	pointer to location of the member where the instance is stored - this is the pointer that will be patched during resolving.
	void ComponentInstance::addToLinkMap(Component* targetResource, const std::string& instancePath, ComponentInstance** targetInstancePtr)
	{
		mLinkMap[targetResource].push_back({ targetInstancePtr, instancePath } );
	}

}
