/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "portal.h"

 // External Includes
#include <entity.h>
#include <nap/core.h>

// nap::Portal run time class definition
RTTI_BEGIN_CLASS(nap::Portal)
RTTI_PROPERTY("WebSocketComponent", &nap::Portal::mWebSocketComponent, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::PortalInstance run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PortalInstance)
RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{

	PortalInstance::~PortalInstance()
	{
	}


	bool PortalInstance::init(utility::ErrorState& errorState)
	{
		return true;
	}
}
