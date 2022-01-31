/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

 // Local Includes
#include "portalitemoperationalcalendar.h"
#include "portalutils.h"

RTTI_BEGIN_CLASS(nap::PortalItemOperationalCalendar)
	RTTI_PROPERTY("Name", &nap::PortalItemOperationalCalendar::mName, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Calendar", &nap::PortalItemOperationalCalendar::mCalendar, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{

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
		auto monday = mCalendar->getInstance().findByTitle<WeeklyCalendarItem>(toString(EDay::Monday));
		auto tuesday = mCalendar->getInstance().findByTitle<WeeklyCalendarItem>(toString(EDay::Tuesday));
		auto wednesday = mCalendar->getInstance().findByTitle<WeeklyCalendarItem>(toString(EDay::Wednesday));
		auto thursday = mCalendar->getInstance().findByTitle<WeeklyCalendarItem>(toString(EDay::Thursday));
		auto friday = mCalendar->getInstance().findByTitle<WeeklyCalendarItem>(toString(EDay::Friday));
		auto saturday = mCalendar->getInstance().findByTitle<WeeklyCalendarItem>(toString(EDay::Saturday));
		auto sunday = mCalendar->getInstance().findByTitle<WeeklyCalendarItem>(toString(EDay::Sunday));
		assert(
			monday != nullptr &&
			tuesday != nullptr &&
			wednesday != nullptr &&
			thursday != nullptr &&
			friday != nullptr &&
			saturday != nullptr &&
			sunday != nullptr
		);

		return std::vector<std::string>({
			monday->getTime().toString(),
			monday->getDuration().toString(),
			tuesday->getTime().toString(),
			tuesday->getDuration().toString(),
			wednesday->getTime().toString(),
			wednesday->getDuration().toString(),
			thursday->getTime().toString(),
			thursday->getDuration().toString(),
			friday->getTime().toString(),
			friday->getDuration().toString(),
			saturday->getTime().toString(),
			saturday->getDuration().toString(),
			sunday->getTime().toString(),
			sunday->getDuration().toString()
		});
	}


	bool nap::PortalItemOperationalCalendar::setCalendarTimes(const std::vector<std::string>& times, utility::ErrorState& error)
	{
		if (!error.check(times.size() == 14, "%s: expected 14 time entries, received %i", mID.c_str(), times.size()))
			return false;

		auto monday = mCalendar->getInstance().findByTitle<WeeklyCalendarItem>(toString(EDay::Monday));
		auto tuesday = mCalendar->getInstance().findByTitle<WeeklyCalendarItem>(toString(EDay::Tuesday));
		auto wednesday = mCalendar->getInstance().findByTitle<WeeklyCalendarItem>(toString(EDay::Wednesday));
		auto thursday = mCalendar->getInstance().findByTitle<WeeklyCalendarItem>(toString(EDay::Thursday));
		auto friday = mCalendar->getInstance().findByTitle<WeeklyCalendarItem>(toString(EDay::Friday));
		auto saturday = mCalendar->getInstance().findByTitle<WeeklyCalendarItem>(toString(EDay::Saturday));
		auto sunday = mCalendar->getInstance().findByTitle<WeeklyCalendarItem>(toString(EDay::Sunday));
		assert(
			monday != nullptr &&
			tuesday != nullptr &&
			wednesday != nullptr &&
			thursday != nullptr &&
			friday != nullptr &&
			saturday != nullptr &&
			sunday != nullptr
		);

		monday->setTime(CalendarItem::Time(times.at(0)));
		monday->setDuration(CalendarItem::Time(times.at(1)));
		tuesday->setTime(CalendarItem::Time(times.at(2)));
		tuesday->setDuration(CalendarItem::Time(times.at(3)));
		wednesday->setTime(CalendarItem::Time(times.at(4)));
		wednesday->setDuration(CalendarItem::Time(times.at(5)));
		thursday->setTime(CalendarItem::Time(times.at(6)));
		thursday->setDuration(CalendarItem::Time(times.at(7)));
		friday->setTime(CalendarItem::Time(times.at(8)));
		friday->setDuration(CalendarItem::Time(times.at(9)));
		saturday->setTime(CalendarItem::Time(times.at(10)));
		saturday->setDuration(CalendarItem::Time(times.at(11)));
		sunday->setTime(CalendarItem::Time(times.at(12)));
		sunday->setDuration(CalendarItem::Time(times.at(13)));
		return mCalendar->getInstance().save(error);
	}
}
