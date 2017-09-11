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
    bool ComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
    {
        return true;
    }
}
