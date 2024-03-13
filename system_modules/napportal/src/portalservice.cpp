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
	{ }


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
		// Check if the portal component is registered and listens to the server
		PortalComponentInstance* portal = findComponentById(event.getPortalID());
		if (portal == nullptr || &portal->getServer() != &server)
			return;

		// Process event with proper method in portal component
		utility::ErrorState error;
		switch (event.getType())
		{
		case EPortalEventType::Request:
			portal->processRequest(event, error);
			break;

		case EPortalEventType::ValueUpdate:
			portal->processUpdate(event, error);
			break;

		default:
			error.fail("Cannot process events with type %s", getPortalEventTypeString(event.getType()).c_str());
			break;
		}

		// Log any errors that occured
		if (error.hasErrors())
			nap::Logger::error(error.toString().c_str());
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
		mComponents.emplace_back(&component);
	}


	void PortalService::removeComponent(PortalComponentInstance& component)
	{
		auto found_it = std::find_if(mComponents.begin(), mComponents.end(), [&](const auto& it)
		{
			return it == &component;
		});
		assert(found_it != mComponents.end());
		mComponents.erase(found_it);
	}


	PortalComponentInstance* PortalService::findComponentById(const std::string& id)
	{
		auto found_it = std::find_if(mComponents.begin(), mComponents.end(), [&](const auto& it)
		{
			return it->getComponent()->mID == id;
		});
		return found_it == mComponents.end() ? nullptr : *found_it;
	}
}
