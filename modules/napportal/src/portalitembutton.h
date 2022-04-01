/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

 // Local Includes
#include "portalitem.h"

// External Includes
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
		virtual bool processUpdate(const APIEvent& event, utility::ErrorState& error) override;

		/**
		 * @return the descriptor of the portal item as an API event
		 */
		virtual APIEventPtr getDescriptor() const override;

		/**
		 * @return the current value of the portal item as an API event
		 */
		virtual APIEventPtr getValue() const override;

		ResourcePtr<ParameterButton> mParameter;	///< Property: 'Parameter' the parameter linked to this portal item
	};
}
