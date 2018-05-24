/// local includes
#include <sandboxservice.h>
#include "nap/windowresource.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SandboxService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	SandboxService::SandboxService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}

}


