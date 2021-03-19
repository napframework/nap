/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "calendaritem.h"

// External includes
#include <nap/event.h>

namespace nap
{
	/**
	 * Occurs when a calendar event starts or ends
	 */
	class NAPAPI CalendarEvent : public Event
	{
		RTTI_ENABLE(Event)
	public:
		/**
		 * @param item the item associated with the event
		 */
		CalendarEvent(const nap::CalendarItem& item) : 
			mItem(&item)								{ }

		/**
		 * @return item associated with this event
		 */
		const nap::CalendarItem& getItem() const		{ return *mItem; }

	private:
		const nap::CalendarItem* mItem = nullptr;
	};
}