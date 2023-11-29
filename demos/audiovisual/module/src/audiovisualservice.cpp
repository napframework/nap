// Local Includes
#include "audiovisualservice.h"

// External Includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <iostream>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audiovisualService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	bool audiovisualService::init(nap::utility::ErrorState& errorState)
	{
		//Logger::info("Initializing audiovisualService");
		return true;
	}


	void audiovisualService::update(double deltaTime)
	{
	}
	

	void audiovisualService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
	}
	

	void audiovisualService::shutdown()
	{
	}
}
