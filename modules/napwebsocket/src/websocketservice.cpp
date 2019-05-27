// Local Includes
#include "websocketservice.h"

// External Includes
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WebsocketService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	WebsocketService::WebsocketService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


    bool WebsocketService::init(nap::utility::ErrorState& errorState)
    {
		return true;
    }    
}
