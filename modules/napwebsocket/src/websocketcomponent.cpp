/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "websocketcomponent.h"
#include "websocketservice.h"

// External Includes
#include <entity.h>
#include <nap/core.h>

// nap::websocketcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::WebSocketComponent)
	RTTI_PROPERTY("Interface", &nap::WebSocketComponent::mInterface, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::websocketcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WebSocketComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	WebSocketComponentInstance::~WebSocketComponentInstance()
	{
		if (mService != nullptr)
			mService->removeComponent(*this);
	}


	bool WebSocketComponentInstance::init(utility::ErrorState& errorState)
	{
		// Extract the server
		mInterface = getComponent<WebSocketComponent>()->mInterface.get();

		mService = getEntityInstance()->getCore()->getService<nap::WebSocketService>();
		assert(mService != nullptr);
		mService->registerComponent(*this);

		return true;
	}
}