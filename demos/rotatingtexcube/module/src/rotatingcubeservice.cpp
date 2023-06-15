// Local Includes
#include "rotatingcubeservice.h"

// External Includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <iostream>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::rotatingcubeService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	bool rotatingcubeService::init(nap::utility::ErrorState& errorState)
	{
		//Logger::info("Initializing rotatingcubeService");
		return true;
	}


	void rotatingcubeService::update(double deltaTime)
	{
	}
	

	void rotatingcubeService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
	}
	

	void rotatingcubeService::shutdown()
	{
	}
}
