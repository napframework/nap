#include "nap/componentinstance.h"
#include "nap/entityinstance.h"

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::ComponentInstance, nap::EntityInstance&)
RTTI_END_CLASS

RTTI_BEGIN_BASE_CLASS(nap::ComponentResource)
RTTI_END_CLASS

namespace nap
{
	ComponentInstance::ComponentInstance(EntityInstance& entity) :
		mEntity(&entity)
	{
	} 
}