/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "portalservice.h"
#include "portalcomponent.h"

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


	PortalService::~PortalService()
	{
	}


	bool PortalService::sendEvent(PortalEventPtr event, const PortalWebSocketComponentInstance& wsComponent, utility::ErrorState& error)
	{
		// Make sure components don't get pulled / updated while forwarding calls
		std::lock_guard<std::mutex> lock(mComponentMutex);

		// Iterate over all the portal components to find the target
		for (const auto& comp : mComponents)
		{
			bool portalIdMatch = comp->getComponent()->mID == event->getPortalID();
			bool wsCompMatch = comp->mWebSocketComponent == &wsComponent;
			if (portalIdMatch && wsCompMatch)
				return comp->processEvent(std::move(event), error);
		}

		// No portal component accepted the event
		const char* serviceName = this->get_type().get_name().data();
		const char* portalId = event->getPortalID().c_str();
		const char* wsCompId = wsComponent.getComponent()->mID.c_str();
		return error.check(false, "%s: No portal with ID \"%s\" accepts events from WebSocket component \"%s\"", serviceName, portalId, wsCompId);
	}


	void PortalService::registerComponent(PortalComponentInstance& component)
	{
		std::lock_guard<std::mutex> lock(mComponentMutex);
		mComponents.emplace_back(&component);
	}


	void PortalService::removeComponent(PortalComponentInstance& component)
	{
		std::lock_guard<std::mutex> lock(mComponentMutex);
		auto found_it = std::find_if(mComponents.begin(), mComponents.end(), [&](const auto& it)
		{
			return it == &component;
		});
		assert(found_it != mComponents.end());
		mComponents.erase(found_it);
	}
}
