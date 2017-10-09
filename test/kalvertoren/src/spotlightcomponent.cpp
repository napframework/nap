#include "spotlightcomponent.h"

// External Includes
#include <nap/entity.h>

RTTI_BEGIN_CLASS(nap::SpotlightComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SpotlightComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS