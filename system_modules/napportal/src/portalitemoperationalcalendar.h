/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

 // Local Includes
#include "portalitem.h"

// External Includes
#include <apievent.h>
#include <operationalcalendar.h>

namespace nap
{
	/**
	 * Represents an operational calendar item in a NAP portal.
	 */
	class NAPAPI PortalItemOperationalCalendar : public PortalItem
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

		std::string mName;							///< Property: 'Name' the name of the calendar in the portal
		ResourcePtr<OperationalCalendar> mCalendar;	///< Property: 'Calendar' the calendar linked to this portal item
    protected:
        /**
         * init
         * @param error contains the error message when initialization fails
         * @return if initialization succeeded.
         */
        bool onInit(utility::ErrorState& errorState) override;
	private:

		/**
		 * @return the times of the linked operational calendar as a vector of strings
		 */
		const std::vector<std::string> getCalendarTimes() const;

		/**
		 * Sets the times of the linked operational calendar from a vector of strings
		 * @param times time specifications
		 * @param error contains the error if the time could not be set
		 * @return if the times are updated
		 */
		bool setCalendarTimes(const std::vector<std::string>& times, utility::ErrorState& error);
	};
}
