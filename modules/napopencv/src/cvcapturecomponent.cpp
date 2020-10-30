/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "cvcapturecomponent.h"
#include "cvservice.h"

// External Includes
#include <entity.h>
#include <entity.h>
#include <nap/core.h>

// nap::cvcapturecomponent run time class definition 
RTTI_BEGIN_CLASS(nap::CVCaptureComponent)
	RTTI_PROPERTY("Devices", &nap::CVCaptureComponent::mDevice, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::cvcapturecomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CVCaptureComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	CVCaptureComponentInstance::~CVCaptureComponentInstance()
	{
		if (mService)
		{
			mService->removeCaptureComponent(*this);
		}
	}


	bool CVCaptureComponentInstance::init(utility::ErrorState& errorState)
	{
		// Copy over capture device
		nap::CVCaptureComponent* resource = getComponent<nap::CVCaptureComponent>();
		mDevice = resource->mDevice.get();

		// Register with service
		mService = getEntityInstance()->getCore()->getService<CVService>();
		assert(mService != nullptr);
		mService->registerCaptureComponent(*this);

		return true;
	}


	const nap::CVCaptureDevice& CVCaptureComponentInstance::getDevice() const
	{
		assert(mDevice != nullptr);
		return *mDevice;
	}


	nap::CVCaptureDevice& CVCaptureComponentInstance::getDevice()
	{
		assert(mDevice != nullptr);
		return *mDevice;
	}


	void CVCaptureComponentInstance::trigger(const nap::CVFrameEvent& frameEvent)
	{
		frameReceived.trigger(frameEvent);
	}

}