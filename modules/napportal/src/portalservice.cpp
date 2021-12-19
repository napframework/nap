/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "portalservice.h"
#include "portalcomponent.h"
#include "portalwebsocketserver.h"
#include "portalutils.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PortalService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{

	PortalService::PortalService(ServiceConfiguration* configuration) : Service(configuration)
	{
	}


	nap::WebSocketService& PortalService::getWebSocketService()
	{
		assert(mWebSocketService != nullptr);
		return *mWebSocketService;
	}


	const nap::WebSocketService& PortalService::getWebSocketService() const
	{
		assert(mWebSocketService != nullptr);
		return *mWebSocketService;
	}


	void PortalService::registerObjectCreators(rtti::Factory& factory)
	{
		factory.addObjectCreator(std::make_unique<PortalWebSocketServerObjectCreator>(*this));
	}


	void PortalService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
		dependencies.emplace_back(RTTI_OF(WebSocketService));
	}


	void PortalService::created()
	{
		mWebSocketService = getCore().getService<WebSocketService>();
		assert(mWebSocketService != nullptr);
	}


	void PortalService::update(double deltaTime)
	{
		std::queue<PortalEventPtr> events;
		for (const auto& server : mServers)
		{
			server->consumePortalEvents(events);
			while (!(events.empty()))
			{
				processEvent(*events.front(), *server);
				events.pop();
			}
		}
	}


	void PortalService::processEvent(PortalEvent& event, PortalWebSocketServer& server)
	{
		// Check if the portal ID exists
		const std::string& portal_id = event.getPortalID();
		if (mComponents.count(portal_id) == 0)
			return;

		// Check if the portal component listens to the server
		const auto& portal = mComponents.at(portal_id);
		if (&portal->getServer() != &server)
			return;

		// Process event with proper method in portal component
		switch (event.getType())
		{
		case EPortalEventType::Request:
			portal->processRequest(event);
			break;

		case EPortalEventType::Update:
			portal->processUpdate(event);
			break;

		default:
			nap::Logger::error("Cannot process events with type %s", getPortalEventTypeString(event.getType()).c_str());
			break;
		}
	}


	void PortalService::registerServer(PortalWebSocketServer& server)
	{
		mServers.emplace_back(&server);
	}
	 

	void PortalService::removeServer(PortalWebSocketServer& server)
	{
		auto found_it = std::find_if(mServers.begin(), mServers.end(), [&](const auto& it)
		{
			return it == &server;
		});
		assert(found_it != mServers.end());
		mServers.erase(found_it);
	}


	void PortalService::registerComponent(PortalComponentInstance& component)
	{
		const std::string& id = component.getComponent()->mID;
		assert(mComponents.count(id) == 0);
		mComponents.emplace(std::make_pair(id, &component));
	}


	void PortalService::removeComponent(PortalComponentInstance& component)
	{
		const std::string& id = component.getComponent()->mID;
		assert(mComponents.count(id) == 1);
		mComponents.erase(id);
	}
}
