/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <apievent.h>

namespace nap
{
	/**
	 * Represents a single item (e.g. slider, toggle, button) in a NAP portal.
	 * Implementations are in derived classes, PortalItem only serves as a base class.
	 */
	class NAPAPI PortalItem : public Resource
	{
		RTTI_ENABLE(Resource)

	public:

		/**
		 * Processes an API event. Implementation differs per derived class
		 * @param event The event to be processed
		 * @param error contains information when processing fails
		 * @return if the event was processed successfully
		 */
		virtual bool processEvent(const APIEvent& event, utility::ErrorState& error) = 0;
	};
}
