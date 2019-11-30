/// local includes
#include "cvservice.h"

// external includes
#include <opencv2/core/ocl.hpp>
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CVService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	CVService::CVService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


	void CVService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
	}


	bool CVService::init(nap::utility::ErrorState& errorState)
	{
		return true;
	}


	void CVService::update(double deltaTime)
	{
	}


	void CVService::registerObjectCreators(rtti::Factory& factory)
	{
	}
}

