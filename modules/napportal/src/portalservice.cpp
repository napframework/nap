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
}
