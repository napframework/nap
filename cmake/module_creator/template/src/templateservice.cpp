// Local Includes
#include "@UNPREFIXED_MODULE_NAME_LOWERCASE@service.h"

// External Includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <iostream>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::@UNPREFIXED_MODULE_NAME_INPUTCASE@Service)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	bool @UNPREFIXED_MODULE_NAME_INPUTCASE@Service::init(nap::utility::ErrorState& errorState)
	{
		//Logger::info("Initializing @UNPREFIXED_MODULE_NAME_INPUTCASE@Service");
		return true;
	}


	void @UNPREFIXED_MODULE_NAME_INPUTCASE@Service::update(double deltaTime)
	{
	}
	

	void @UNPREFIXED_MODULE_NAME_INPUTCASE@Service::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
	}
	

	void @UNPREFIXED_MODULE_NAME_INPUTCASE@Service::shutdown()
	{
	}
}
