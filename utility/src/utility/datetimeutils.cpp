#pragma once

// Local Includes
#include "datetimeutils.h"

// External Includes
#include <ctime>
#include <assert.h>
#include <sstream>
#include <iomanip>

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
			return DateTime(getCurrentTime());
		}


		extern void getCurrentDateTime(DateTime& outDateTime)
		{
			outDateTime.setTimeStamp(getCurrentTime());
		}
		

		DateTime::DateTime(const SystemTimeStamp& timeStamp, ConversionMode mode) : mMode(mode)
		{
			setTimeStamp(timeStamp);
		}


		DateTime::DateTime(const SystemTimeStamp& timeStamp) : mMode(ConversionMode::Local)
		{
			setTimeStamp(timeStamp);
		}


		void DateTime::setTimeStamp(const SystemTimeStamp& timeStamp)
		{
			mTimeStamp = timeStamp;
			auto ctime = utility::SystemClock::to_time_t(mTimeStamp);
			switch (mMode)
			{
			case DateTime::ConversionMode::Local:
				mTimeStruct = *(std::localtime(&ctime));
			case ConversionMode::GMT:
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


		int DateTime::getMonth() const
		{
			return mTimeStruct.tm_mon + 1;
		}


		int DateTime::getDay() const
		{
			return mTimeStruct.tm_mday;
		}


		int DateTime::getDayInTheYear() const
		{
			return mTimeStruct.tm_yday;
		}


		int DateTime::getDayInTheWeek() const
		{
			return mTimeStruct.tm_wday;
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
			std::ostringstream oss;
			oss << std::put_time(&mTimeStruct, "%d-%m-%Y %H-%M-%S");
			return oss.str();
		}


		bool DateTime::isDaylightSaving() const
		{
			return mTimeStruct.tm_isdst > 0;
		}

	}
}