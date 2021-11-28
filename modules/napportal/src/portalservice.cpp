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


	void PortalService::registerPortalComponent(PortalComponentInstance& portalComponent)
	{
		std::lock_guard<std::mutex> lock(mPortalComponentMutex);
		mPortalComponents.emplace_back(&portalComponent);
	}


	void PortalService::removePortalComponent(PortalComponentInstance& portalComponent)
	{
		std::lock_guard<std::mutex> lock(mPortalComponentMutex);
		auto found_it = std::find_if(mPortalComponents.begin(), mPortalComponents.end(), [&](const auto& it)
		{
			return it == &portalComponent;
		});
		assert(found_it != mPortalComponents.end());
		mPortalComponents.erase(found_it);
	}
}
