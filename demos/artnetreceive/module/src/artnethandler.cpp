/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "artnethandler.h"

 // External includes
#include <entity.h>
#include <artnetinputcomponent.h>
#include <algorithm>

// Register Art-Net Handler Component
RTTI_BEGIN_CLASS(nap::ArtNetHandlerComponent)
	RTTI_PROPERTY("Input", &nap::ArtNetHandlerComponent::mInput, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// Register Art-Net Handler Component Instance
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ArtNetHandlerComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{

	bool ArtNetHandlerComponentInstance::init(utility::ErrorState& errorState)
	{
		mInput->packetReceived.connect(eventReceivedSlot);
		return true;
	}


	const std::vector<float>& ArtNetHandlerComponentInstance::getData()
	{
		return mReceivedData;
	}


	void ArtNetHandlerComponentInstance::onEventReceived(const ArtNetEvent& event)
	{
		mReceivedData.clear();

		// Store data as float so we can render it directly in the ImGui histogram
		for (int16_t i = 0; i < event.getChannelCount(); i++)
			mReceivedData.emplace_back(static_cast<float>(event[i]));
	}
}
