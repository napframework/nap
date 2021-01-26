#include "calendaritem.h"

// nap::calendaritem run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CalendarItem)
	RTTI_PROPERTY("Time",			&nap::CalendarItem::mTime,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Duration",		&nap::CalendarItem::mDuration,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Title",			&nap::CalendarItem::mTitle,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Description",	&nap::CalendarItem::mDescription,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::MonthlyCalendarItem)
	RTTI_PROPERTY("Day",			&nap::MonthlyCalendarItem::mDay,			nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::WeeklyCalendarItem)
	RTTI_PROPERTY("Day",			&nap::WeeklyCalendarItem::mDay,				nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::DailyCalendarItem)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniqueCalendarItem)
	RTTI_PROPERTY("Date", &nap::UniqueCalendarItem::mDate, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS



//////////////////////////////////////////////////////////////////////////

/**
 * Returns if the given date exists
 * @param m month
 * @param d day
 * @param y year
 */
static bool dateExists(int m, int d, int y)
{
	//Gregorian dates started in 1582
	if (!(1582 <= y))
		return false;
	if (!(1 <= m && m <= 12))
		return false;
	if (!(1 <= d && d <= 31))
		return false;
	if ((d == 31) && (m == 2 || m == 4 || m == 6 || m == 9 || m == 11))
		return false;
	if ((d == 30) && (m == 2))
		return false;
	if ((m == 2) && (d == 29) && (y % 4 != 0))
		return false;
	if ((m == 2) && (d == 29) && (y % 400 == 0))
		return true;
	if ((m == 2) && (d == 29) && (y % 100 == 0))
		return false;
	if ((m == 2) && (d == 29) && (y % 4 == 0))
		return true;

	return true;
}

namespace nap
{

	bool CalendarItem::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(
			mTime.x >= 0 && mTime.x < 24 && 
			mTime.y >= 0 && mTime.y < 60, 
			"%s, invalid time", mID.c_str()))
			return false;

		if (!errorState.check(mDuration.x >= 0 && mDuration.y >= 0,
			"%s, negative duration", mID.c_str()))
			return false;

		return true;
	}


	bool MonthlyCalendarItem::init(utility::ErrorState& errorState)
	{
		if (!CalendarItem::init(errorState))
			return false;

		// Check if day is at least within bounds
		if (!errorState.check(mDay >= 1 && mDay <= 31, "%s: invalid day", mID.c_str()))
			return false;

		return true;
	}


	bool WeeklyCalendarItem::init(utility::ErrorState& errorState)
	{
		if (!CalendarItem::init(errorState))
			return false;

		// Ensure day is not specified as unknown
		if (!errorState.check(mDay != EDay::Unknown, "%s: day not specified", mID.c_str()))
			return false;

		return CalendarItem::init(errorState);
	}


	bool DailyCalendarItem::init(utility::ErrorState& errorState)
	{
		return CalendarItem::init(errorState);
	}


	bool UniqueCalendarItem::init(utility::ErrorState& errorState)
	{
		// Base class
		if (!CalendarItem::init(errorState))
			return false;

		// Ensure date exists
		if(!errorState.check(dateExists(static_cast<int>(mDate.mMonth), mDate.mDay, mDate.mYear), 
			"%s: invalid date", mID.c_str()))
			return false;

		return true;
	}
}