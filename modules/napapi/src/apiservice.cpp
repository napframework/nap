// Local Includes
#include "apiservice.h"
#include "apievent.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APIService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{

	APIService::APIService(ServiceConfiguration* configuration) : Service(configuration)
	{
		std::cout << "blaat";
	}

	APIService::~APIService()
	{

	}


	bool APIService::sendFloat(const char* action, float value)
	{
		APICallEventPtr ptr = std::make_unique<APICallEvent>(std::move(std::string(action)));
		ptr->addArgument<APIFloat>(value);
		return true;
	}


	bool APIService::sendString(const char* action, const char* value)
	{
		APICallEventPtr ptr = std::make_unique<APICallEvent>(std::move(std::string(action)));
		ptr->addArgument<APIString>(std::move(std::string(value)));
		return true;
	}


	bool APIService::init(utility::ErrorState& error)
	{
		return true;
	}


	void APIService::shutdown()
	{

	}
}