#include "instanceptr.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::InstancePtrBase)
RTTI_END_CLASS

namespace nap
{
	// Empty constructor needed to make sure this cpp is not optimized away, causing the RTTI definition to go missing
	InstancePtrBase::InstancePtrBase()
	{
	}
}
