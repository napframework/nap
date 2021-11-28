/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "portalcomponent.h"

 // External Includes
#include <entity.h>
#include <nap/core.h>
#include <nap/logger.h>

// nap::PortalComponent run time class definition
RTTI_BEGIN_CLASS(nap::PortalComponent)
RTTI_PROPERTY("PortalAPIComponent", &nap::PortalComponent::mPortalAPIComponent, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::PortalComponentInstance run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PortalComponentInstance)
RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{

	bool PortalComponentInstance::init(utility::ErrorState& errorState)
	{
		// Register with the service
		mService = getEntityInstance()->getCore()->getService<nap::PortalService>();
		assert(mService != nullptr);
		mService->registerPortalComponent(*this);

		return true;
	}


	void PortalComponentInstance::onDestroy()
	{
		// De-register with the service
		if (mService != nullptr)
			mService->removePortalComponent(*this);
	}


	void PortalComponentInstance::processPortalRequest(const WebSocketConnection& connection)
	{

	}


	void PortalComponentInstance::processPortalUpdate(const std::vector<APIMessage*>& messages)
	{

	}
}
