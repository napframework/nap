// Local Includes
#include "websocketservice.h"
#include "websocketserver.h"

// External Includes
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WebSocketService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	WebSocketService::WebSocketService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


    bool WebSocketService::init(nap::utility::ErrorState& errorState)
    {
		return true;
    }    


	void WebSocketService::registerObjectCreators(rtti::Factory& factory)
	{
		factory.addObjectCreator(std::make_unique<WebSocketServerObjectCreator>(*this));
	}
}
