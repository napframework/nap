/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

 // Local Includes
#include "portalitem.h"
#include "portalutils.h"

// External Includes
#include <apivalue.h>
#include <apievent.h>
#include <parameterbutton.h>

namespace nap
{
	/**
	 * Represents a button in a NAP portal.
	 */
	class PortalItemButton : public PortalItem
	{
		RTTI_ENABLE(PortalItem)

	public:

		/**
		 * Processes an update type API event.
		 * @param event The event to be processed
		 * @param error contains information when processing fails
		 * @return if the event was processed successfully
		 */
		virtual bool processUpdate(const APIEvent& event, utility::ErrorState& error) override
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
				if (!mParameter->isPressed())
				{
					mLastEvent = EPortalItemButtonEvent::Press;
					mParameter->setPressed(true);
				}
				return true;
			case EPortalItemButtonEvent::Release:
				if (mParameter->isPressed())
				{
					mLastEvent = EPortalItemButtonEvent::Release;
					mParameter->setPressed(false);
				}
				return true;
			case EPortalItemButtonEvent::Click:
				mLastEvent = EPortalItemButtonEvent::Click;
				mParameter->click();
				return true;
			default:
				return error.check(false, "%s: received invalid portal item button event \"%s\"", mID.c_str(), value.c_str());
			}
		};

		/**
		 * @return the descriptor of the portal item as an API event
		 */
		virtual APIEventPtr getDescriptor() const override
		{
			// Send an "Invalid" event value with the descriptor, so the client can deduce the argument type
			std::string value = getPortalItemButtonEventString(EPortalItemButtonEvent::Invalid);
			APIEventPtr event = std::make_unique<APIEvent>(mParameter->getDisplayName(), mID);
			event->addArgument<APIString>(nap::portal::itemTypeArgName, get_type().get_name().data());
			event->addArgument<APIString>(nap::portal::itemValueArgName, value);
			return event;
		};

		/**
		 * @return the current value of the portal item as an API event
		 */
		virtual APIEventPtr getValue() const override
		{
			APIEventPtr event = std::make_unique<APIEvent>(mParameter->getDisplayName(), mID);
			event->addArgument<APIString>(nap::portal::itemValueArgName, getPortalItemButtonEventString(mLastEvent));
			return event;
		};

		ResourcePtr<ParameterButton> mParameter;	///< Property: 'Parameter' the parameter linked to this portal item

	private:

		EPortalItemButtonEvent mLastEvent;			///< The last event that was triggered by this portal item button
	};
}
