// Local includes
#include "oscinputcomponent.h"
#include "oscservice.h"

// External includes
#include <nap/entity.h>
#include <nap/core.h>
#include <iostream>

RTTI_BEGIN_CLASS(nap::OSCInputComponent)
	RTTI_PROPERTY("Addresses", &nap::OSCInputComponent::mAddressFilter, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OSCInputComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
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


	bool OSCInputComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
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