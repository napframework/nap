/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "datetime.h"

// External Includes
#include <assert.h>
#include <iomanip>
#include <iostream>
#include <unordered_map>
#include <utility/stringutils.h>

RTTI_BEGIN_ENUM(nap::EDay)
	RTTI_ENUM_VALUE(nap::EDay::Monday,		"Monday"),
	RTTI_ENUM_VALUE(nap::EDay::Tuesday,		"Tuesday"),
	RTTI_ENUM_VALUE(nap::EDay::Wednesday,	"Wednesday"),
	RTTI_ENUM_VALUE(nap::EDay::Thursday,	"Thursday"),
	RTTI_ENUM_VALUE(nap::EDay::Friday,		"Friday"),
	RTTI_ENUM_VALUE(nap::EDay::Saturday,	"Saturday"),
	RTTI_ENUM_VALUE(nap::EDay::Sunday,		"Sunday"),
	RTTI_ENUM_VALUE(nap::EDay::Unknown,		"Unknown")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(nap::EMonth)
	RTTI_ENUM_VALUE(nap::EMonth::January,	"January"),
	RTTI_ENUM_VALUE(nap::EMonth::February,	"February"),
	RTTI_ENUM_VALUE(nap::EMonth::March,		"March"),
	RTTI_ENUM_VALUE(nap::EMonth::April,		"April"),
	RTTI_ENUM_VALUE(nap::EMonth::May,		"May"),
	RTTI_ENUM_VALUE(nap::EMonth::June,		"June"),
	RTTI_ENUM_VALUE(nap::EMonth::July,		"July"),
	RTTI_ENUM_VALUE(nap::EMonth::August,	"August"),
	RTTI_ENUM_VALUE(nap::EMonth::September, "September"),
	RTTI_ENUM_VALUE(nap::EMonth::October,	"October"),
	RTTI_ENUM_VALUE(nap::EMonth::November,	"November"),
	RTTI_ENUM_VALUE(nap::EMonth::December,	"December"),
	RTTI_ENUM_VALUE(nap::EMonth::Unknown,	"Unknown")
RTTI_END_ENUM

RTTI_BEGIN_STRUCT(nap::Date)
	RTTI_VALUE_CONSTRUCTOR(const nap::SystemTimeStamp&)
	RTTI_PROPERTY("Month",	&nap::Date::mMonth,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Day",	&nap::Date::mDay,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Year",	&nap::Date::mYear,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::TimeStamp)
	RTTI_PROPERTY("Time", &nap::TimeStamp::mTimeStamp, nap::rtti::EPropertyMetaData::Default)
	RTTI_VALUE_CONSTRUCTOR(const nap::SystemTimeStamp&)
RTTI_END_STRUCT

namespace nap
{
	SystemTimeStamp getCurrentTime()
	{
		return SystemClock::now();
	}


	DateTime getCurrentDateTime()
	{
		return DateTime(SystemClock::now(), DateTime::ConversionMode::Local);
	}


	void getCurrentDateTime(DateTime& outDateTime)
	{
		outDateTime.setTimeStamp(getCurrentTime());
	}


	std::string timeFormat(const SystemTimeStamp& time, const std::string& format)
	{
		// TODO: This can be optimized, doing a little too much here now
		DateTime dt(time);
		std::string msstring = utility::stringFormat("%03d", dt.getMilliSecond());
		std::string format_no_ms = utility::replaceAllInstances(format, "%ms", msstring);
		time_t tt = std::chrono::system_clock::to_time_t(time);
		char fmtstr[256];
		std::strftime(fmtstr, sizeof(fmtstr), format_no_ms.c_str(), std::localtime(&tt));
		return std::string(fmtstr);
	}

	SystemTimeStamp createTimestamp(int year, int month, int day, int hour, int minute, int second, int millisecond, bool daylightSaving)
	{
		std::tm tm{};
		tm.tm_year = year - 1900;
		tm.tm_mon = month - 1;
		tm.tm_mday = day;
		tm.tm_hour = hour;
		tm.tm_min = minute;
		tm.tm_sec = second;
		tm.tm_isdst = daylightSaving ? -1 : 0;
		time_t time = std::mktime(&tm);
		return std::chrono::system_clock::from_time_t(time) + std::chrono::milliseconds(millisecond);
	}


	const std::array<nap::EDay, 7>& getDaysInWeek()
	{
		const static std::array<EDay, 7> days = {
			EDay::Monday,
			EDay::Tuesday,
			EDay::Wednesday,
			EDay::Thursday,
			EDay::Friday,
			EDay::Saturday,
			EDay::Sunday
		};
		return days;
	}


	std::string toString(EDay day)
	{
		rtti::TypeInfo enum_type = RTTI_OF(EDay);
		return enum_type.get_enumeration().value_to_name(day).to_string();
	}


	EDay toDay(const std::string& string)
	{
		rtti::TypeInfo enum_type = RTTI_OF(EDay);
		rtti::Variant var = enum_type.get_enumeration().name_to_value(string.data());
		return var.is_valid() ? var.get_value<EDay>() : EDay::Unknown;
	}


	EMonth toMonth(const std::string& string)
	{
		rtti::TypeInfo enum_type = RTTI_OF(EMonth);
		rtti::Variant var = enum_type.get_enumeration().name_to_value(string.data());
		return var.is_valid() ? var.get_value<EMonth>() : EMonth::Unknown;
	}


	std::string toString(EMonth month)
	{
		rtti::TypeInfo enum_type = RTTI_OF(EMonth);
		return enum_type.get_enumeration().value_to_name(month).to_string();
	}


	DateTime::DateTime(const SystemTimeStamp& timeStamp, ConversionMode mode) : mMode(mode)
	{
		setTimeStamp(timeStamp);
	}


	DateTime::DateTime(const SystemTimeStamp& timeStamp) : mMode(ConversionMode::Local)
	{
		setTimeStamp(timeStamp);
	}


	DateTime::DateTime()
	{
		setTimeStamp(SystemClock::now());
	}


	void DateTime::setTimeStamp(const SystemTimeStamp& timeStamp)
	{
		mTimeStamp = timeStamp;
		auto ctime = SystemClock::to_time_t(mTimeStamp);
		switch (mMode)
		{
		case DateTime::ConversionMode::Local:
			mTimeStruct = *(std::localtime(&ctime));
			break;
		case DateTime::ConversionMode::GMT:
			mTimeStruct = *(std::gmtime(&ctime));
			break;
		default:
			assert(false);
			break;
		}
	}


	int DateTime::getYear() const
	{
		return mTimeStruct.tm_year + 1900;
	}


	int DateTime::getHour() const
	{
		return mTimeStruct.tm_hour;
	}


	EMonth DateTime::getMonth() const
	{
		return static_cast<EMonth>(mTimeStruct.tm_mon+1);
	}


	int DateTime::getWeek() const
	{
		// Calculating the weeknumber for a date can be complicated (stuff to do with what the first Thursday of a year is, etc).
		// See http://www.boyet.com/articles/publishedarticles/calculatingtheisoweeknumb.html
		// strftime can do it for us, so lets just use it (though it's not the most efficient...)
		char buffer[16] = { 0 };
		strftime(buffer, 16, "%W", &mTimeStruct);

		return atoi(buffer);
	}


	int DateTime::getDayInTheMonth() const
	{
		return mTimeStruct.tm_mday;
	}


	int DateTime::getDayInTheYear() const
	{
		return mTimeStruct.tm_yday;
	}


	EDay DateTime::getDay() const
	{
		return static_cast<EDay>(mTimeStruct.tm_wday);
	}


	int DateTime::getMinute() const
	{
		return mTimeStruct.tm_min;
	}


	int DateTime::getSecond() const
	{
		return mTimeStruct.tm_sec;
	}


	int DateTime::getMilliSecond() const
	{
		// Convert time stamp to seconds, causing it to loose milliseconds
		auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(mTimeStamp);

		// Get the difference
		auto fraction = mTimeStamp - seconds;

		// And voila
		return std::chrono::duration_cast<std::chrono::milliseconds>(fraction).count();
	}


	std::string DateTime::toString() const
	{
		std::string rv = asctime(&mTimeStruct);
		rv.pop_back();
		return rv;
	}


	bool DateTime::isDaylightSaving() const
	{
		return mTimeStruct.tm_isdst > 0;
	}


	Date::Date(const SystemTimeStamp& systemTime) 
	{
		fromSystemTime(systemTime);
	}


	Date::Date(int year, EMonth month, int day) :
		mYear(year), mMonth(month), mDay(day)
	{ }


	void Date::fromSystemTime(const SystemTimeStamp& systemTime)
	{
		DateTime date_time(systemTime);
		mMonth = date_time.getMonth();
		mDay = date_time.getDayInTheMonth() ;
		mYear = date_time.getYear();
	}


	bool nap::Date::exists(int y, EMonth m, int d)
	{
		if (!(1 <= (int)m && (int)m <= 12))
			return false;
		if (!(1 <= d && d <= 31))
			return false;
		if ((d == 31) && ((int)m == 2 || (int)m == 4 || (int)m == 6 || (int)m == 9 || (int)m == 11))
			return false;
		if ((d == 30) && ((int)m == 2))
			return false;
		if (((int)m == 2) && (d == 29) && (y % 4 != 0))
			return false;
		if (((int)m == 2) && (d == 29) && (y % 400 == 0))
			return true;
		if (((int)m == 2) && (d == 29) && (y % 100 == 0))
			return false;
		if (((int)m == 2) && (d == 29) && (y % 4 == 0))
			return true;

		return true;
	}


	bool nap::Date::valid() const
	{
		return Date::exists(mYear, mMonth, mDay);
	}


	SystemTimeStamp Date::toSystemTime() const
	{
		assert(this->valid());
		return createTimestamp(mYear, static_cast<int>(mMonth), mDay, 0, 0);
	}


	TimeStamp::TimeStamp(const SystemTimeStamp& systemTime)
	{
		fromSystemTime(systemTime);
	}


	void TimeStamp::fromSystemTime(const SystemTimeStamp& systemTime)
	{
		// construct time_point based on system_clock, but with the precision of milliseconds instead of whatever precision your system_clock has.
		auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(systemTime);

		// store epoch is in milliseconds as long
		mTimeStamp = ms.time_since_epoch().count();
	}


	nap::SystemTimeStamp TimeStamp::toSystemTime() const
	{
		std::chrono::milliseconds dur(mTimeStamp);
		return std::chrono::time_point<std::chrono::system_clock>(dur);
	}
}
