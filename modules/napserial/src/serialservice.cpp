// Local Includes
#include "serialservice.h"

// External includes
#include <serial/serial.h>
#include <memory>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SerialService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	SerialService::SerialService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}

	SerialService::~SerialService()
	{
	}


	bool SerialService::init(utility::ErrorState& error)
	{
		return true;
	}


	void SerialService::shutdown()
	{
	}
}