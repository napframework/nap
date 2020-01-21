// Local Includes
#include "cvcapturecomponent.h"
#include "cvservice.h"

// External Includes
#include <entity.h>
#include <entity.h>
#include <nap/core.h>

// nap::cvcapturecomponent run time class definition 
RTTI_BEGIN_CLASS(nap::CVCaptureComponent)
	RTTI_PROPERTY("Devices", &nap::CVCaptureComponent::mDevices, nap::rtti::EPropertyMetaData::Default)
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
		// Copy over all valid devices to receive opencv events from
		nap::CVCaptureComponent* resource = getComponent<nap::CVCaptureComponent>();
		mDevices.reserve(resource->mDevices.size());
		for (auto& device : resource->mDevices)
		{
			mDevices.emplace(device.get());
		}

		// Register with service
		mService = getEntityInstance()->getCore()->getService<CVService>();
		assert(mService != nullptr);
		mService->registerCaptureComponent(*this);

		return true;
	}


	void CVCaptureComponentInstance::update(double deltaTime)
	{

	}


	void CVCaptureComponentInstance::trigger(const nap::CVFrameEvent& frameEvent)
	{
		frameReceived.trigger(frameEvent);
	}

}