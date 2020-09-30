// Local Includes
#include "apiwebsocketservice.h"
#include "apiwebsocketserver.h"
#include "apiwebsocketclient.h"

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


	nap::WebSocketService& APIWebSocketService::getWebSocketService()
	{
		assert(mWebSocketService != nullptr);
		return *mWebSocketService;
	}


	const nap::WebSocketService& APIWebSocketService::getWebSocketService() const
	{
		assert(mWebSocketService != nullptr);
		return *mWebSocketService;
	}


	void APIWebSocketService::registerObjectCreators(rtti::Factory& factory)
	{
		factory.addObjectCreator(std::make_unique<APIWebSocketServerObjectCreator>(*this));
		factory.addObjectCreator(std::make_unique<APIWebSocketClientObjectCreator>(*this));
	}


	void APIWebSocketService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
		dependencies.emplace_back(RTTI_OF(APIService));
		dependencies.emplace_back(RTTI_OF(WebSocketService));
	}


	void APIWebSocketService::created()
	{
		mAPIService = getCore().getService<APIService>();
		assert(mAPIService != nullptr);

		mWebSocketService = getCore().getService<WebSocketService>();
		assert(mWebSocketService != nullptr);
	}
}
