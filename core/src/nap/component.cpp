#include <rtti/pythonmodule.h>
#include "nap/component.h"
#include "nap/entity.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Component)
RTTI_END_CLASS

namespace nap
{
    bool ComponentInstance::init(const ObjectPtr<Component>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
    {
        return true;
    }
}
