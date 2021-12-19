/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "portalservice.h"
#include "portalcomponent.h"
#include "portalwebsocketserver.h"

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
