// Local Includes
#include "yoctoservice.h"

// External includes
#include <yocto_api.h>

RTTI_DEFINE_CLASS(nap::YoctoService)

namespace nap
{
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