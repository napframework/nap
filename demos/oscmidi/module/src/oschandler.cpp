/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "oschandler.h"

// External includes
#include <entity.h>
#include <oscinputcomponent.h>

// Register OSC Handler Component
RTTI_BEGIN_CLASS(nap::OscHandlerComponent, "Converts incoming osc messages to a string and stores them")
RTTI_END_CLASS

// Register OSC Handler Component Instance
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OscHandlerComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    
    void OscHandlerComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
    {
        components.emplace_back(RTTI_OF(nap::OSCInputComponent));

    }

    
    bool OscHandlerComponentInstance::init(utility::ErrorState& errorState)
    {
		mReceivedEvents.reserve(25);

		OSCInputComponentInstance* osc_input = getEntityInstance()->findComponent<OSCInputComponentInstance>();
		if (!errorState.check(osc_input != nullptr, "%s: missing OSCInputComponent", mID.c_str()))
			return false;

        osc_input->messageReceived.connect(eventReceivedSlot);
        return true;
    }
    
    
    const std::vector<std::string>& OscHandlerComponentInstance::getMessages()
    {
		return mReceivedEvents;
    }

    
    void OscHandlerComponentInstance::onEventReceived(const OSCEvent& event)
    {
		// Convert and push back the message
		std::string message(event.getAddress());
		for (const auto& argument : event.getArguments())
			message.append(" " + argument->toString());

		// Add
		mReceivedEvents.emplace_back(message);

		// Remove first element when out of range
		if (mReceivedEvents.size() > 25)
		{
			mReceivedEvents.erase(mReceivedEvents.begin());
		}
    }
}
