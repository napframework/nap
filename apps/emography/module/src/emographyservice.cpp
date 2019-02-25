// Local Includes
#include "emographyservice.h"
#include "datamodel.h" 

// External Includes
#include <glm/glm.hpp>
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <sceneservice.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::EmographyService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	EmographyService::EmographyService(ServiceConfiguration* configuration) :
		Service(configuration)
	{ }


	bool EmographyService::init(nap::utility::ErrorState& errorState)
	{
		return true;
	}


	void EmographyService::shutdown()
	{
	}


	void EmographyService::registerObjectCreators(rtti::Factory& factory)
	{
		factory.addObjectCreator(std::make_unique<emography::DataModelObjectCreator>(*this));
	}
}
