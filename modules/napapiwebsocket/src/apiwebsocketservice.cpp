// Local Includes
#include "apiwebsocketservice.h"
#include "apiwebsocketserver.h"
#include "apiwebsocketdispatcher.h"

// External Includes
#include <nap/logger.h>
#include <nap/core.h>
#include <websocketservice.h>

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
		mAPIService = getCore().getService<APIService>();
		assert(mAPIService != nullptr);
		return true;
    }    


	nap::APIService& APIWebSocketService::getAPIService()
	{
		assert(mAPIService != nullptr);
		return *mAPIService;
	}


	const nap::APIService& APIWebSocketService::getAPIService() const
	{
		assert(mAPIService != nullptr);
		return *mAPIService;
	}


	void APIWebSocketService::registerObjectCreators(rtti::Factory& factory)
	{
		factory.addObjectCreator(std::make_unique<APIWebSocketServerObjectCreator>(*this));
		factory.addObjectCreator(std::make_unique<APIWebSocketDispatcherObjectCreator>(*this));
	}


	void APIWebSocketService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
		dependencies.emplace_back(RTTI_OF(APIService));
		dependencies.emplace_back(RTTI_OF(WebSocketService));
	}

}
