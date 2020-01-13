// Local Includes
#include "soemservice.h"

// External includes
#include <memory>
#include <nap/logger.h>
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
		// Print adapter info
		nap::Logger::info("SOEM: available network adapters: ");
		ec_adaptert* it_adapter = ec_find_adapters();

		int id(0);
		while (it_adapter != NULL)
		{
			nap::Logger::info("%d: Device: %s, Description: %s", id, it_adapter->name, it_adapter->desc);
			it_adapter = it_adapter->next;
			id++;
		}
		return true;
	}
}