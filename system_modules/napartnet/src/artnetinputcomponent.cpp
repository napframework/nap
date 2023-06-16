/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "artnetinputcomponent.h"
#include "artnetservice.h"

// External includes
#include <nap/core.h>

RTTI_BEGIN_CLASS(nap::ArtNetInputComponent)
	RTTI_PROPERTY("Net", &nap::ArtNetInputComponent::mNet, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SubNet", &nap::ArtNetInputComponent::mSubNet, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Universe", &nap::ArtNetInputComponent::mUniverse, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Receive All", &nap::ArtNetInputComponent::mReceiveAll, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ArtNetInputComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
	RTTI_FUNCTION("getPacketReceived", &nap::ArtNetInputComponentInstance::getPacketReceived)
RTTI_END_CLASS

namespace nap
{
	ArtNetInputComponentInstance::~ArtNetInputComponentInstance()
	{
		if (mService != nullptr)
			mService->removeInputComponent(*this);
	}


	bool ArtNetInputComponentInstance::init(utility::ErrorState& errorState)
	{
		// Copy settings
		mNet = getComponent<ArtNetInputComponent>()->mNet;
		mSubNet = getComponent<ArtNetInputComponent>()->mSubNet;
		mUniverse = getComponent<ArtNetInputComponent>()->mUniverse;
		mReceiveAll = getComponent<ArtNetInputComponent>()->mReceiveAll;

		// Get service and register
		mService = getEntityInstance()->getCore()->getService<ArtNetService>();
		assert(mService != nullptr);
		mService->addInputComponent(*this);

		return true;
	}


	void ArtNetInputComponentInstance::trigger(const nap::ArtNetEvent& event)
	{
		packetReceived(event);
	}
}
