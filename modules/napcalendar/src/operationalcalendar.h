/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "calendar.h"

// External Includes
#include <nap/resource.h>

namespace nap
{
	/**
	 * Special, but common, type of calendar.
	 * Allows you to define and inspect, for every day of the week, when something is in operation.
	 * If you want more control use the regular nap::Calendar instead.
	 * The title of every created item is the 'nap::EDay' of the week, the ID is unique.
	 */
	class NAPAPI OperationalCalendar : public ICalendar
	{
		RTTI_ENABLE(ICalendar)
	public:
		// Constructor
		OperationalCalendar(nap::Core& core);

		// Destructor
		virtual ~OperationalCalendar();

		/**
		* Creates and initializes the operational calendar instance
		* @return if initialization succeeded
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		* @return the calendar instance, only available after initialization
		*/
		CalendarInstance& getInstance() override						{ assert(mInstance != nullptr);  return *mInstance; }

		/**
		* @return the calendar instance, only available after initialization
		*/
		const CalendarInstance& getInstance() const override			{ assert(mInstance != nullptr);  return *mInstance; }

		/**
		 * @return if, for any day of the week, an item is currently active.
		 */
		bool isOperational();

		bool mAllowFailure = true;			///< Property: 'AllowLoadFailure' If initialization continues when loading a calendar from disk fails. In that case resource defaults are used.
		CalendarItem::Point mMonday;		///< Property: 'Monday' Monday operational hours
		CalendarItem::Point mTuesday;		///< Property: 'Tuesday' Tuesday operational hours
		CalendarItem::Point mWednesday;		///< Property: 'Wednesday' Wednesday operational hours
		CalendarItem::Point mThursday;		///< Property: 'Thursday' Thursday operational hours
		CalendarItem::Point mFriday;		///< Property: 'Friday' Friday operational hours
		CalendarItem::Point mSaturday;		///< Property: 'Saturday' Saturday operational hours
		CalendarItem::Point mSunday;		///< Property: 'Sunday' Sunday operational hours

	private:

		/**
		 * Adds an item to list if not null
		 * @param item item to add
		 * @param list of items to add to.
		 */
		void addItem(const CalendarItem::Point& point, EDay day, CalendarItemList& outItems);

		std::unique_ptr<CalendarInstance> mInstance = nullptr;	///< Calendar runtime instance
	};
}
