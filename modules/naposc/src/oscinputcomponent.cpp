// Local includes
#include "oscinputcomponent.h"
#include "oscservice.h"

// External includes
#include <nap/entity.h>
#include <nap/core.h>
#include <iostream>

RTTI_BEGIN_CLASS(nap::OSCInputComponent)
	RTTI_PROPERTY("Addresses", &nap::OSCInputComponent::mAddresses, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OSCInputComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	OSCInputComponentInstance::~OSCInputComponentInstance()
	{
		nap::OSCService* service = getEntityInstance()->getCore()->getService<OSCService>();
		assert(service != nullptr);
		service->removeInputComponent(*this);
	}


	bool OSCInputComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// Copy addresses
		mAddresses = getComponent<OSCInputComponent>()->mAddresses;

		// Get service and register
		nap::OSCService* service = getEntityInstance()->getCore()->getService<OSCService>();
		assert(service != nullptr);
		service->registerInputComponent(*this);

		return true;
	}


	void OSCInputComponentInstance::trigger(const nap::OSCEvent& oscEvent)
	{
		messageReceived(oscEvent);
	}

}