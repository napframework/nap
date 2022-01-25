/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

 // Local Includes
#include "portalitembutton.h"

RTTI_BEGIN_CLASS(nap::PortalItemButton)
	RTTI_PROPERTY("Parameter", &nap::PortalItemButton::mParameter, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{

	bool PortalItemButton::processUpdate(const APIEvent& event, utility::ErrorState& error)
	{
		// Check for the portal item value argument
		const APIArgument* arg = event.getArgumentByName(nap::portal::itemValueArgName);
		if (!error.check(arg != nullptr, "%s: update event missing argument %s", mID.c_str(), nap::portal::itemValueArgName))
			return false;

		// Check the portal item value type
		const rtti::TypeInfo type = arg->getValueType();
		if (!error.check(type == RTTI_OF(std::string), "%s: cannot process value type %s", mID.c_str(), type.get_name().data()))
			return false;

		// Retrieve the button event and call the corresponding action on the parameter
		std::string value = static_cast<const APIString*>(&arg->getValue())->mValue;
		EPortalItemButtonEvent button_event = getPortalItemButtonEvent(value);
		switch (button_event)
		{
		case EPortalItemButtonEvent::Press:
			mParameter->setPressed(true);
			return true;

		case EPortalItemButtonEvent::Release:
			mParameter->setPressed(false);
			return true;

		case EPortalItemButtonEvent::Click:
			mParameter->click();
			return true;

		default:
			return error.check(false, "%s: received invalid portal item button event \"%s\"", mID.c_str(), value.c_str());
		}
	};


	APIEventPtr PortalItemButton::getDescriptor() const
	{
		// Send an "Invalid" event value with the descriptor, so the client can deduce the argument type
		std::string value = getPortalItemButtonEventString(EPortalItemButtonEvent::Invalid);
		APIEventPtr event = std::make_unique<APIEvent>(mParameter->getDisplayName(), mID);
		event->addArgument<APIString>(nap::portal::itemTypeArgName, get_type().get_name().data());
		event->addArgument<APIString>(nap::portal::itemValueArgName, value);
		return event;
	}


	APIEventPtr PortalItemButton::getValue() const
	{
		// Send an "Invalid" event value, because clients shouln't be interested in button events
		std::string value = getPortalItemButtonEventString(EPortalItemButtonEvent::Invalid);
		APIEventPtr event = std::make_unique<APIEvent>(mParameter->getDisplayName(), mID);
		event->addArgument<APIString>(nap::portal::itemValueArgName, value);
		return event;
	}
}
