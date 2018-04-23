// Local Includes
#include "@MODULE_NAME_LOWERCASE@service.h"

// External Includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <iostream>

RTTI_DEFINE_CLASS(nap::@MODULE_NAME_PASCALCASE@Service)

namespace nap
{
	bool @MODULE_NAME_PASCALCASE@Service::init(nap::utility::ErrorState& errorState)
	{
		//Logger::info("Initializing @MODULE_NAME_PASCALCASE@Service");
		return true;
	}

	void @MODULE_NAME_PASCALCASE@Service::update(double deltaTime)
	{
	}
	
	void @MODULE_NAME_PASCALCASE@Service::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
	}
	
	void @MODULE_NAME_PASCALCASE@Service::shutdown()
	{
	}
}
