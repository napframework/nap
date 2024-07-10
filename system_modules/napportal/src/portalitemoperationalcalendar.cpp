/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

 // Local Includes
#include "portalitemoperationalcalendar.h"
#include "portalutils.h"

// External Includes
#include <array>

RTTI_BEGIN_CLASS(nap::PortalItemOperationalCalendar, "Operational calendar portal item")
	RTTI_PROPERTY("Name",		&nap::PortalItemOperationalCalendar::mName,		nap::rtti::EPropertyMetaData::Required, "Calendar name")
	RTTI_PROPERTY("Calendar",	&nap::PortalItemOperationalCalendar::mCalendar, nap::rtti::EPropertyMetaData::Required, "Calendar resource")
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{
    bool PortalItemOperationalCalendar::onInit(utility::ErrorState& error)
    {
        mDisplayName = mName;
        return true;
    }


	bool PortalItemOperationalCalendar::processUpdate(const APIEvent& event, utility::ErrorState& error)
	{
		// Check for the portal item value argument
		const APIArgument* arg = event.getArgumentByName(nap::portal::itemValueArgName);
		if (!error.check(arg != nullptr, "%s: update event missing argument %s", mID.c_str(), nap::portal::itemValueArgName))
			return false;

		// Ensure that the argument is an array
		if (!error.check(arg->isArray(), "%s: expected array for value argument, not %s", mID.c_str(), arg->getValueType().get_name().data()))
			return false;

		// Cast the argument to an array of strings
		const std::vector<std::string>* times = arg->asArray<std::string>();
		if (!error.check(times != nullptr, "%s: expected array of strings for value argument, not %s", mID.c_str(), arg->getValueType().get_name().data()))
			return false;

		return setCalendarTimes(*times, error);
	};


	APIEventPtr PortalItemOperationalCalendar::getDescriptor() const
	{
		APIEventPtr event = std::make_unique<APIEvent>(mName, mID);
		event->addArgument<APIString>(nap::portal::itemTypeArgName, get_type().get_name().data());
		event->addArgument<APIStringArray>(nap::portal::itemValueArgName, getCalendarTimes());
        addStateArguments(event);
        return event;
	}


	APIEventPtr PortalItemOperationalCalendar::getValue() const
	{
		APIEventPtr event = std::make_unique<APIEvent>(mName, mID);
		event->addArgument<APIStringArray>(nap::portal::itemValueArgName, getCalendarTimes());
        return event;
	}


	const std::vector<std::string> nap::PortalItemOperationalCalendar::getCalendarTimes() const
	{
		const auto& days = getDaysInWeek();
		std::vector<std::string> times;
		times.reserve(days.size() * 2);
		for (const auto& day : days)
		{
			auto calendar_item = mCalendar->getInstance().findByTitle<WeeklyCalendarItem>(toString(day));
			assert(calendar_item != nullptr);
			times.emplace_back(calendar_item->getTime().toString());
			times.emplace_back(calendar_item->getDuration().toString());
		}
		return times;
	}


	bool nap::PortalItemOperationalCalendar::setCalendarTimes(const std::vector<std::string>& times, utility::ErrorState& error)
	{
		if (!error.check(times.size() == 14, "%s: expected 14 time entries, received %i", mID.c_str(), times.size()))
			return false;

		const auto& days = getDaysInWeek();
		std::size_t idx = 0;
		for (const auto& day : days)
		{
			auto calendar_item = mCalendar->getInstance().findByTitle<WeeklyCalendarItem>(toString(day));
			assert(calendar_item != nullptr);
			calendar_item->setTime(CalendarItem::Time::fromString(times.at(idx + 0)));
			calendar_item->setDuration(CalendarItem::Time::fromString(times.at(idx + 1)));
			idx += 2;
		}
		return true;
	}
}
