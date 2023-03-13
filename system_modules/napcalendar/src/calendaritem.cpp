/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "calendaritem.h"

RTTI_BEGIN_STRUCT(nap::CalendarItem::Time)
	RTTI_VALUE_CONSTRUCTOR(int, int)
	RTTI_PROPERTY("Hour",	&nap::CalendarItem::Time::mHour,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Minute", &nap::CalendarItem::Time::mMinute,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::CalendarItem::Point)
	RTTI_VALUE_CONSTRUCTOR(nap::CalendarItem::Time, nap::CalendarItem::Time)
	RTTI_PROPERTY("Time",		&nap::CalendarItem::Point::mTime,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Duration",	&nap::CalendarItem::Point::mDuration,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CalendarItem)
	RTTI_PROPERTY("Title",			&nap::CalendarItem::mTitle,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Point",			&nap::CalendarItem::mPoint,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Description",	&nap::CalendarItem::mDescription,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniqueCalendarItem)
	RTTI_PROPERTY("Date", &nap::UniqueCalendarItem::mDate, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::YearlyCalendarItem)
	RTTI_PROPERTY("Day", &nap::YearlyCalendarItem::mDay, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Month", &nap::YearlyCalendarItem::mMonth, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::MonthlyCalendarItem)
	RTTI_PROPERTY("Day",			&nap::MonthlyCalendarItem::mDay,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::WeeklyCalendarItem)
	RTTI_PROPERTY("Day",			&nap::WeeklyCalendarItem::mDay,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::DailyCalendarItem)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{

	CalendarItem::Time::Time(int hour, int minute) : mHour(hour), mMinute(minute)
	{ }


	std::string CalendarItem::Time::toString() const
	{
		return utility::stringFormat("%.2d:%.2d", mHour, mMinute);
	}


	CalendarItem::Time CalendarItem::Time::fromString(const std::string& time)
	{
		Time timestruct = { -1, -1 };
		std::vector<std::string> parts = utility::splitString(time, ':');
		if (parts.size() == 2)
		{
			timestruct.mHour = std::stoi(parts[0]);
			timestruct.mMinute = std::stoi(parts[1]);
		}
		return timestruct;
	}


	nap::CalendarItem::Point::Point(Time time, Time duration) :
		mTime(std::move(time)), 
		mDuration(std::move(duration))
	{ }


	nap::Minutes nap::CalendarItem::Time::toMinutes() const
	{
		return nap::Minutes((mHour * 60) + mMinute);
	}


	bool nap::CalendarItem::Point::valid() const
	{
		return mTime.mHour >= 0 && mTime.mHour < 24 &&
			mTime.mMinute >= 0 && mTime.mMinute < 60;
	}


	CalendarItem::CalendarItem(const Point& point, const std::string& title) :
		mPoint(point), mTitle(title)
	{ }


	bool CalendarItem::init(utility::ErrorState& errorState)
	{
		// Valid time in day
		if (!errorState.check(mPoint.valid(), "%s: invalid time", mID.c_str()))
			return false;
		return true;
	}


	void CalendarItem::setTitle(const std::string& title)
	{
		mTitle = title;
	}


	const std::string& CalendarItem::getTitle() const
	{
		return mTitle;
	}


	void CalendarItem::setDescription(const std::string& description)
	{
		mDescription = description;
	}


	const std::string& CalendarItem::getDescription() const
	{
		return mDescription;
	}


	bool CalendarItem::setPoint(const Point& point)
	{
		if (point.valid())
		{
			mPoint = point;
			return true;
		}
		return false;
	}


	const CalendarItem::Point& CalendarItem::getPoint() const
	{
		return mPoint;
	}


	bool CalendarItem::setTime(const Time& time)
	{
		Time backup = mPoint.mTime;
		mPoint.mTime = time;
		if (!mPoint.valid())
		{
			mPoint.mTime = backup;
			return false;
		}
		return true;
	}


	const nap::CalendarItem::Time& CalendarItem::getTime() const
	{
		return mPoint.mTime;
	}


	void CalendarItem::setDuration(const Time& duration)
	{
		mPoint.mDuration = duration;
	}


	const ::nap::CalendarItem::Time& CalendarItem::getDuration() const
	{
		return mPoint.mDuration;
	}


	WeeklyCalendarItem::WeeklyCalendarItem(const CalendarItem::Point& point, const std::string& title, EDay day) :
		CalendarItem(point, title), mDay(day)
	{ }


	bool WeeklyCalendarItem::init(utility::ErrorState& errorState)
	{
		if (!CalendarItem::init(errorState))
			return false;

		// Ensure day is not specified as unknown
		if (!errorState.check(mDay != EDay::Unknown, "%s: day not specified", mID.c_str()))
			return false;

		return CalendarItem::init(errorState);
	}


	bool WeeklyCalendarItem::setDay(EDay day)
	{
		if (day != EDay::Unknown)
		{
			mDay = day;
			return true;
		}
		return false;
	}


	nap::EDay WeeklyCalendarItem::getDay() const
	{
		return mDay;
	}


	bool WeeklyCalendarItem::active(SystemTimeStamp timeStamp) const
	{
		// Compute start / end in minutes
		Minutes sample_mins(static_cast<int>(mDay) * 24 * 60);
		Minutes sta_time = sample_mins + mPoint.mTime.toMinutes();
		Minutes end_time = sta_time + mPoint.mDuration.toMinutes();

		// Get current minutes in week
		DateTime cur_dt(timeStamp);
		Minutes cur_day_mins(static_cast<int>(cur_dt.getDay()) * 24 * 60);
		Minutes cur_hou_mins((cur_dt.getHour() * 60) + cur_dt.getMinute());
		Minutes cur_mins(cur_day_mins + cur_hou_mins);

		// See if it's in bounds
		if (cur_mins >= sta_time && cur_mins < end_time)
			return true;

		// Wrap to start if end_time exceeds total minutes in week
		constexpr Minutes minutesWeek(7 * 24 * 60);
		if (end_time > minutesWeek)
		{
			Minutes wrapped(end_time.count() % minutesWeek.count());
			return cur_mins < wrapped;
		}
		return false;
	}


	DailyCalendarItem::DailyCalendarItem(const CalendarItem::Point& point, const std::string& title) :
		CalendarItem(point, title)
	{ }


	bool DailyCalendarItem::init(utility::ErrorState& errorState)
	{
		return CalendarItem::init(errorState);
	}


	bool DailyCalendarItem::active(SystemTimeStamp timeStamp) const
	{
		// Get minute bounds for day
		Minutes sta_time(mPoint.mTime.toMinutes());
		Minutes end_time = sta_time + mPoint.mDuration.toMinutes();

		// Get current minute in day
		DateTime cur_dt(timeStamp);
		Minutes cur_mins((cur_dt.getHour() * 60) + cur_dt.getMinute());

		// See if it's in bounds
		if (cur_mins >= sta_time && cur_mins < end_time)
			return true;

		// Wrap to start if end_time exceeds total minutes in day
		constexpr Minutes minutesDay(Hours(24));
		if (end_time > minutesDay)
		{
			Minutes wrapped(end_time.count() % minutesDay.count());
			return cur_mins < wrapped;
		}
		return false;
	}


	UniqueCalendarItem::UniqueCalendarItem(const CalendarItem::Point& point, const std::string& title, const Date& date) :
		CalendarItem(point, title), mDate(date)
	{ }


	bool UniqueCalendarItem::init(utility::ErrorState& errorState)
	{
		// Base class
		if (!CalendarItem::init(errorState))
			return false;

		// Ensure date exists
		if(!errorState.check(mDate.valid(), "%s: invalid date", mID.c_str()))
			return false;

		return true;
	}


	bool UniqueCalendarItem::setDate(const nap::Date& date)
	{
		if (!date.valid()) { return false; }
		mDate = date;
		return true;
	}


	const nap::Date& UniqueCalendarItem::getDate() const
	{
		return mDate;
	}


	bool UniqueCalendarItem::active(SystemTimeStamp timeStamp) const
	{
		SystemTimeStamp sta_time = mDate.toSystemTime() + mPoint.mTime.toMinutes();
		SystemTimeStamp end_time = sta_time + mPoint.mDuration.toMinutes();
		return (timeStamp >= sta_time && timeStamp < end_time);
	}


	MonthlyCalendarItem::MonthlyCalendarItem(const CalendarItem::Point& point, const std::string& title, int day) :
		CalendarItem(point, title), mDay(day)
	{ }


	bool MonthlyCalendarItem::init(utility::ErrorState& errorState)
	{
		if (!CalendarItem::init(errorState))
			return false;

		// Check if day is at least within bounds
		if (!errorState.check(mDay >= 1 && mDay <= 31, "%s: invalid day", mID.c_str()))
			return false;

		return true;
	}


	bool MonthlyCalendarItem::setDay(int day)
	{
		if (day >= 1 && day <= 31)
		{
			mDay = day;
			return true;
		}
		return false;
	}


	bool MonthlyCalendarItem::active(SystemTimeStamp timeStamp) const
	{
		// Get sample (lookup) date
		DateTime cur_dt(timeStamp);
		int cur_year = cur_dt.getYear();
		int cur_mont = static_cast<int>(cur_dt.getMonth());
		
		// If the day exists in this month, we can try to sample ahead.
		if (Date::exists(cur_dt.getYear(), cur_dt.getMonth(), mDay))
		{
			// If current time is ahead of lookup, see if it's in range
			SystemTimeStamp lookup_time = createTimestamp(cur_year, cur_mont, mDay, mPoint.mTime.mHour, mPoint.mTime.mMinute);
			if (timeStamp >= lookup_time)
			{
				SystemTimeStamp end_time = lookup_time + mPoint.mDuration.toMinutes();
				return timeStamp < end_time;
			}
		}

		// Otherwise we need to check if the current time falls within the window of the previous months range.
		// For example: an event triggered at the end of a month can be active in the beginning of a new month.
		cur_mont -= 1;
		if (cur_mont == 0)
		{
			cur_mont = 12;
			cur_year -= 1;
		}

		// Make sure the day exists in the last month, if not the event could not have been triggered
		if (!Date::exists(cur_year, static_cast<EMonth>(cur_mont), mDay))
			return false;

		// Previous month sample date
		SystemTimeStamp lookup_time = createTimestamp(cur_year, cur_mont, mDay, mPoint.mTime.mHour, mPoint.mTime.mMinute);

		// Check if it falls in the previous month window
		SystemTimeStamp end_time = lookup_time + mPoint.mDuration.toMinutes();
		if (timeStamp >= lookup_time && timeStamp < end_time)
			return true;

		return false;
	}


	YearlyCalendarItem::YearlyCalendarItem(const CalendarItem::Point& point, const std::string& title, EMonth month, int day) :
		CalendarItem(point, title), mMonth(month), mDay(day) { }


	bool YearlyCalendarItem::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mDay >= 1 && mDay <= 31, "%s: Invalid day", mID.c_str()))
			return false;

		if (!errorState.check(mMonth != EMonth::Unknown, "%s: Invalid month", mID.c_str()))
			return false;

		return true;
	}


	bool YearlyCalendarItem::setDate(EMonth month, int day)
	{
		if (day < 1 || day > 31 || month == EMonth::Unknown)
			return false;
		mDay = day; mMonth = month;
		return false;
	}


	bool YearlyCalendarItem::active(SystemTimeStamp timeStamp) const
	{
		// Get sample (lookup) date
		DateTime cur_dt(timeStamp);
		int cur_year = cur_dt.getYear();

		// If the date exists in this year, we can try to sample ahead.
		if (Date::exists(cur_dt.getYear(), mMonth, mDay))
		{
			// If current time is ahead of lookup, see if it's in range
			SystemTimeStamp lookup_time = createTimestamp(cur_year, static_cast<int>(mMonth), mDay, mPoint.mTime.mHour, mPoint.mTime.mMinute);
			if (timeStamp >= lookup_time)
			{
				SystemTimeStamp end_time = lookup_time + mPoint.mDuration.toMinutes();
				return timeStamp < end_time;
			}
		}

		// Otherwise we need to check if the current time falls within the window of the previous year range.
		// For example: an event triggered at the end of a year can be active in the beginning of a new year.
		// First make sure the date exists last year, if not the event could not have been triggered
		cur_year -= 1;
		if (!Date::exists(cur_year, static_cast<EMonth>(mMonth), mDay))
			return false;

		// Previous year sample date
		SystemTimeStamp lookup_time = createTimestamp(cur_year, static_cast<int>(mMonth), mDay, mPoint.mTime.mHour, mPoint.mTime.mMinute);

		// Check if it falls in the previous year window
		SystemTimeStamp end_time = lookup_time + mPoint.mDuration.toMinutes();
		if (timeStamp >= lookup_time && timeStamp < end_time)
			return true;

		return false;
	}
}
