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


	void WebSocketService::update(double deltaTime)
	{
		std::queue<WebSocketEventPtr> events;
		for (auto& server : mServers)
		{
			server->consumeEvents(events);
			while (!(events.empty()))
			{
				WebSocketEvent& websocket_event = *(events.front());
				nap::Logger::info("processing: %s", websocket_event.get_type().get_name().to_string().c_str());
				events.pop();
			}
		}
	}


	void WebSocketService::registerServer(WebSocketServer& server)
	{
		mServers.emplace_back(&server);
	}


	void WebSocketService::removeServer(WebSocketServer& server)
	{
		auto found_it = std::find_if(mServers.begin(), mServers.end(), [&](const auto& it)
		{
			return it == &server;
		});
		assert(found_it != mServers.end());
		mServers.erase(found_it);
	}
}
