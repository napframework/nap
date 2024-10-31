/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "midihandler.h"

// External includes
#include <entity.h>
#include <midiinputcomponent.h>

// RTTI
RTTI_BEGIN_CLASS(nap::MidiHandlerComponent, "Converts incoming midi messages to a string and stores them")
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::MidiHandlerComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    
    void MidiHandlerComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
    {
        components.emplace_back(RTTI_OF(nap::MidiInputComponent));

    }

    
    bool MidiHandlerComponentInstance::init(utility::ErrorState& errorState)
    {
		mReceivedEvents.reserve(25);

		MidiInputComponentInstance* midi_input = getEntityInstance()->findComponent<MidiInputComponentInstance>();
		if (!errorState.check(midi_input != nullptr, "%s: missing MidiInputComponent", mID.c_str()))
			return false;

        midi_input->messageReceived.connect(eventReceivedSlot);
        return true;
    }
    
    
    const std::vector<std::string>& MidiHandlerComponentInstance::getMessages()
    {
		return mReceivedEvents;
    }

    
    void MidiHandlerComponentInstance::onEventReceived(const MidiEvent& event)
    {
		mReceivedEvents.emplace_back(event.toString());
		if (mReceivedEvents.size() > 25)
		{
			mReceivedEvents.erase(mReceivedEvents.begin());
		}
    }
}
