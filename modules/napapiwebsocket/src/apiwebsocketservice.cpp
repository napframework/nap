// Local Includes
#include "apiwebsocketservice.h"
#include "apiwebsocketserver.h"

// External Includes
#include <nap/logger.h>
#include <websocketservice.h>
#include <apiservice.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APIWebSocketService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	APIWebSocketService::APIWebSocketService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


    bool APIWebSocketService::init(nap::utility::ErrorState& errorState)
    {
		return true;
    }    


	void APIWebSocketService::registerObjectCreators(rtti::Factory& factory)
	{
		factory.addObjectCreator(std::make_unique<APIWebSocketServerObjectCreator>(*this));
	}


	void APIWebSocketService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
		dependencies.emplace_back(RTTI_OF(APIService));
		dependencies.emplace_back(RTTI_OF(WebSocketService));
	}

}
