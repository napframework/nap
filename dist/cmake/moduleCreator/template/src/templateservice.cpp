// Local Includes
#include "@MODULE_NAME_LOWERCASE@service.h"

// External Includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <iostream>

RTTI_DEFINE(nap::@MODULE_NAME_CAMELCASE@Service)

namespace nap
{
	bool @MODULE_NAME_CAMELCASE@Service::init(nap::utility::ErrorState& errorState)
	{
		Logger::info("@MODULE_NAME_CAMELCASE@Service init");
		return true;
	}

	void @MODULE_NAME_CAMELCASE@Service::update(double deltaTime)
	{
		double current_time = getCore().getElapsedTime();
//		Logger::info("@MODULE_NAME_CAMELCASE@Service update");
	}
	
	void @MODULE_NAME_CAMELCASE@Service::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
	}
}
