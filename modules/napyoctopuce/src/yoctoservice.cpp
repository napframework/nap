// Local Includes
#include "yoctoservice.h"

// External includes
#include <yocto_api.h>
#include <memory>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::YoctoService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	YoctoService::YoctoService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}

	YoctoService::~YoctoService()
	{
		yFreeAPI();
	}


	bool YoctoService::init(utility::ErrorState& error)
	{
		return true;
	}


	void YoctoService::shutdown()
	{
	}
}