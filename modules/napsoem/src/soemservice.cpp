// Local Includes
#include "soemservice.h"

// External includes
#include <memory>
#include <soem/ethercat.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SOEMService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	SOEMService::SOEMService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}

	SOEMService::~SOEMService()
	{
	}


	bool SOEMService::init(utility::ErrorState& error)
	{
		return true;
	}


	void SOEMService::shutdown()
	{
	}
}