#include "componentptr.h"
#include "component.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ComponentPtrBase)
RTTI_END_CLASS

namespace nap
{
	// Regular ptr Ctor
	ComponentPtrBase::ComponentPtrBase(Component* resource) :
		InstancePtrBase(resource)
	{
	}
}