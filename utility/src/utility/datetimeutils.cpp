// Local Includes
#include "datetimeutils.h"
#include "stringutils.h"

// External Includes
#include <assert.h>
#include <iomanip>
#include <iostream>
#include <unordered_map>

namespace nap
{
	namespace utility
	{
		SystemTimeStamp getCurrentTime()
		{
			return SystemClock::now();
		}

		extern DateTime getCurrentDateTime()
		{
			return DateTime(getCurrentTime(), DateTime::ConversionMode::Local);
		}


		extern void getCurrentDateTime(DateTime& outDateTime)
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

		SystemTimeStamp createTimestamp(int year, int month, int day, int hour, int minute, int second, int millisecond)
		{
			std::tm tm{};
			tm.tm_year = year - 1900;
			tm.tm_mon = month - 1;
			tm.tm_mday = day;
			tm.tm_hour = hour;
			tm.tm_min = minute;
			tm.tm_sec = second;
			time_t time = std::mktime(&tm);
			return std::chrono::system_clock::from_time_t(time) + std::chrono::milliseconds(millisecond);
		}

		using DayToStringMap = std::unordered_map<EDay, std::string>;
		static const DayToStringMap& getDaysToStringMap()
		{
			static DayToStringMap map;
			if (map.empty())
			{
				map[EDay::Monday]		= "Monday";
				map[EDay::Tuesday]		= "Tuesday";
				map[EDay::Wednesday]	= "Wednesday";
				map[EDay::Thursday]		= "Thursday";
				map[EDay::Friday]		= "Friday";
				map[EDay::Saturday]		= "Saturday";
				map[EDay::Sunday]		= "Sunday";
			}
			return map;
		}


		extern std::string toString(EDay day)
		{
			const DayToStringMap& map = getDaysToStringMap();
			auto it = map.find(day);
			assert(it != map.end());
			return it->second;
		}


		extern EDay toDay(const std::string& string)
		{
			const DayToStringMap& map = getDaysToStringMap();
			for (const auto& kv : map)
			{
				if (kv.second == string)
					return kv.first;
			}
			return EDay::Unknown;
		}


		using MonthsToStringMap = std::unordered_map<EMonth, std::string>;
		static const MonthsToStringMap& getMonthsToStringMap()
		{
			static MonthsToStringMap map;
			if (map.empty())
			{
				map[EMonth::January]	= "January";
				map[EMonth::February]	= "February";
				map[EMonth::March]		= "March";
				map[EMonth::April]		= "April";
				map[EMonth::May]		= "May";
				map[EMonth::June]		= "June";
				map[EMonth::July]		= "July";
				map[EMonth::August]		= "August";
				map[EMonth::September]	= "September";
				map[EMonth::October]	= "October";
				map[EMonth::November]	= "November";
				map[EMonth::December]	= "December";
			}
			return map;
		}


		extern EMonth toMonth(const std::string& string)
		{
			const MonthsToStringMap& map = getMonthsToStringMap();
			for (const auto& kv : map)
			{
				if (kv.second == string)
					return kv.first;
			}
			return EMonth::Unknown;
		}


		extern std::string toString(EMonth month)
		{
			const MonthsToStringMap& map = getMonthsToStringMap();
			auto it = map.find(month);
			assert(it != map.end());
			return it->second;
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
			auto ctime = utility::SystemClock::to_time_t(mTimeStamp);
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
			return static_cast<EMonth>(mTimeStruct.tm_mon);
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
			return std::string(asctime(&mTimeStruct));
		}


		bool DateTime::isDaylightSaving() const
		{
			return mTimeStruct.tm_isdst > 0;
		}
	}
}