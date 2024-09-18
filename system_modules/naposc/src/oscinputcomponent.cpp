/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "oscinputcomponent.h"
#include "oscservice.h"

// External includes
#include <entity.h>
#include <nap/core.h>
#include <iostream>

RTTI_BEGIN_CLASS(nap::OSCInputComponent, "Receives OSC events")
	RTTI_PROPERTY("Addresses", &nap::OSCInputComponent::mAddressFilter, nap::rtti::EPropertyMetaData::Default, "OSC addresses this component is allowed to receive, empty = all")
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OSCInputComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
    RTTI_FUNCTION("getMessageReceived", &nap::OSCInputComponentInstance::getMessageReceived)
RTTI_END_CLASS

namespace nap
{
	OSCInputComponentInstance::~OSCInputComponentInstance()
	{
		if (mService != nullptr)
		{
			mService->removeInputComponent(*this);
		}
	}


	bool OSCInputComponentInstance::init(utility::ErrorState& errorState)
	{
		// Copy addresses
		mAddressFilter = getComponent<OSCInputComponent>()->mAddressFilter;

		// Get service and register
		mService = getEntityInstance()->getCore()->getService<OSCService>();
		assert(mService != nullptr);
		mService->registerInputComponent(*this);

		return true;
	}


	void OSCInputComponentInstance::trigger(const nap::OSCEvent& oscEvent)
	{
		messageReceived(oscEvent);
	}

}
