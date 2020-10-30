/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <chrono>
#include <stdint.h>
#include <string>
#include <ctime>
#include <functional>
#include <utility/dllexport.h>
#include <rtti/typeinfo.h>

namespace nap
{
	// Typedefs
	using SystemClock = std::chrono::system_clock;							///< System clock, able to convert time points in to days, seconds etc.
	using HighResolutionClock = std::chrono::high_resolution_clock;			///< High resolution clock, works with the highest possible precision. Can't convert time points in to days, seconds etc.
	using Milliseconds = std::chrono::milliseconds;							///< Milliseconds type definition
	using MicroSeconds = std::chrono::microseconds;							///< Microseconds type definition
	using NanoSeconds = std::chrono::nanoseconds;							///< Nanoseconds type definition
	using Seconds = std::chrono::seconds;									///< Seconds type definition
	using Minutes = std::chrono::minutes;									///< Minutes type definition
	using Hours = std::chrono::hours;										///< Hours type definition
	using SystemTimeStamp = std::chrono::time_point<SystemClock>;			///< Point in time associated with the SystemClock
	using HighResTimeStamp = std::chrono::time_point<HighResolutionClock>;	///< Point in time associated with the HighResolutionClock

	// Forward declares
	class DateTime;


	//////////////////////////////////////////////////////////////////////////
	// Utility
	//////////////////////////////////////////////////////////////////////////

	/**
	 * @return the current time as a system time stamp, this time is acquired using the system clock.
	 * This time can be converted in days, minutes etc using a nap::DateTime object.
	 */
	NAPAPI SystemTimeStamp getCurrentTime();

	/**
	 * Returns the current system data / time.
	 * Note that the time will be Local to this computer and includes daylight savings.
	 * @return a structure that contains the current date and time.
	 */
	NAPAPI DateTime	getCurrentDateTime();

	/**
	 * Populates a DateTime structure that contains the current date and time
	 * Note that the time will be Local to this computer and includes daylight savings
	 * @param outDateTime the time structure to populate with the current date and time
	 */
	NAPAPI void	getCurrentDateTime(DateTime& outDateTime);

	/**
	 * Convert a timestamp to string using a string format simlar to strftime.
	 * Also takes care of milliseconds using %ms
	 * @param time the timestamp to format into a string
	 * @param format the strftime-like format string
	 * @return the formatted date / time string
	 */
	NAPAPI std::string timeFormat(const SystemTimeStamp& time, const std::string& format = "%Y-%m-%d %H:%M:%S.%ms");

	/**
	 * Create a timestamp from the given data
	 * @param year the year as number (eg. 1970)
	 * @param month the month as 1-based number (3 == march)
	 * @param day the day of the month ranging from 1 to 31
	 * @param hour the hour of the day from 0 to 23
	 * @param minute the minute of the hour from 0 to 59
	 * @param second the second of the minute from 0 to 60
	 * @param millisecond additional milliseconds
	 * @return the complete timestamp
	 */
	NAPAPI SystemTimeStamp createTimestamp(int year, int month, int day, int hour, int minute, int second=0, int millisecond=0);


	//////////////////////////////////////////////////////////////////////////
	// Day
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Convenience enum that describes the day of the week.
	 * This enum is serializable.
	 */
	enum class EDay : int
	{
		Monday		= 1,		///< Monday
		Tuesday		= 2,		///< Tuesday
		Wednesday	= 3,		///< Wednesday
		Thursday	= 4,		///< Thursday
		Friday		= 5,		///< Friday
		Saturday	= 6,		///< Saturday
		Sunday		= 0,		///< Sunday
		Unknown		= -1		///< Unknown
	};

	/**
	 * Converts a day in to a string
	 * @param day the day to convert in to a string 
	 * @return the day as a string
	 */
	NAPAPI std::string toString(EDay day);

	/**
	* Converts a string in to a day. This call is case-sensitive
	* @param string the name of the day, see EDay comments for string representation
	* @return the converted month, Unknown if not valid match is found.
	*/
	NAPAPI EDay toDay(const std::string& string);


	//////////////////////////////////////////////////////////////////////////
	// Month
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Convenience enum that describes the month in a year.
	 * This enum is serializable.
	 */
	enum class EMonth : int
	{
		January		= 1,		///< January
		February	= 2,		///< February
		March		= 3,		///< March
		April		= 4,		///< April
		May			= 5,		///< May
		June		= 6,		///< June
		July		= 7,		///< July
		August		= 8,		///< August
		September	= 9,		///< September
		October		= 10,		///< October
		November	= 11,		///< November
		December	= 12,		///< December
		Unknown		= -1		///< Unknown
	};
	
	/**
	* Converts a month in to a string
	* @param month the month to convert in to a string
	* @return the month as a string
	*/
	NAPAPI std::string toString(EMonth month);

	/**
	 * Converts a string in to a month. This call is case-sensitive
	 * @param string the name of the month, see EMonth comments for string representation
	 * @return the converted month, Unknown if not valid match is found.
	 */
	NAPAPI EMonth toMonth(const std::string& string);


	//////////////////////////////////////////////////////////////////////////
	// DateTime
	//////////////////////////////////////////////////////////////////////////

	/**
 	 * Represents a point in time based on a system time stamp. 
	 * This is a run-time only class that wraps a system time stamp for easier readability and use. 
	 * To actually save (serialize) or read (deserialize) a particular point in time use a nap::TimeStamp or nap::Date object.
	 */
	class NAPAPI DateTime final
	{
	public:

		/**
		* Specifies the way a timestamp is interpreted
		*/
		enum class ConversionMode : int
		{
			Local = 0,		///< Local time, including possible daylight saving adjustment
			GMT = 1			///< Greenwich Mean Time, excluding daylight saving adjustment
		};

	public:
		/**
		* The object is constructed using the system's local date and time
		*/
		DateTime();

		/**
		* @param timeStamp the time that defines this object's date and time
		* @param mode the way time is interpreted, local includes possible daylight savings, GMT does not
		*/
		DateTime(const SystemTimeStamp& timeStamp, ConversionMode mode);

		/**
		* When using this constructor time is interpreted as Local to the computer and includes daylight saving adjustments
		* @param timeStamp the time that defines this object's date and time
		*/
		DateTime(const SystemTimeStamp& timeStamp);

		/**
		*	Default destructor, the timestamp will be
		*/
		~DateTime() = default;

		/**
		* @return the year associated with the time stamp
		*/
		int getYear() const;

		/**
		* @return months since January (0, 11) as a EMonth where Sunday is 0 and Monday is 1
		*/
		EMonth getMonth() const;

		/**
		* @return The ISO-8601 weeknumber: weeks start with Monday. Week 1 for a year is the week that contains the first Thursday for that year.
		*/
		int getWeek() const;

		/**
		* @return day of the month (1,31)
		*/
		int getDayInTheMonth() const;

		/**
		* @return the day since january first (0,365)
		*/
		int getDayInTheYear() const;

		/**
		*	@return the day in the week since sunday (0,6) as a EDay
		*/
		EDay getDay() const;

		/**
		* @return the hour since midnight (0, 23)
		*/
		int getHour() const;

		/**
		* @return the minute after the hour (0,59)
		*/
		int getMinute() const;

		/**
		* @return second after the minute (0,60)
		*/
		int getSecond() const;

		/**
		* @return the milliseconds associated with the time stamp
		*/
		int getMilliSecond() const;

		/**
		*	@return if this date time object takes in to account daylight savings
		*/
		bool isDaylightSaving() const;

		/**
		* @return human readable string representation of the date and time
		*/
		std::string toString() const;

		/**
		* Sets the time stamp that is used to define this object's date and time
		* @param timeStamp the new TimeStamp
		*/
		void setTimeStamp(const SystemTimeStamp& timeStamp);

		/**
		*	@return the time stamp asscoiated with this object
		*/
		const SystemTimeStamp& getTimeStamp() const { return mTimeStamp; }

	private:

		SystemTimeStamp		mTimeStamp = SystemClock::now();	///< The timestamp that contains all the timing information
		std::tm				mTimeStruct;						///< Extracted c style struct
		ConversionMode		mMode = ConversionMode::Local;		///< Current time conversion mode
	};


	//////////////////////////////////////////////////////////////////////////
	// Date
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Serializable date structure that can be converted into a system time stamp.
	 * Use this when serialization / de-serialization of a date is required.
	 * This is a relatively light weight object that can be both copy and move constructed or assigned.
	 */
	class NAPAPI Date final
	{
		RTTI_ENABLE()
	public:
		/**
		 * Default constructor
		 */
		Date() = default;

		/**
		 * Constructor based on given system time. Extracts the date from the time stamp.
		 * @param systemTime the time stamp to extract the date from. 
		 */
		Date(const SystemTimeStamp& systemTime);

		/**
		 * Extracts a date from a system time stamp
		 * @param systemTime the time stamp to extract the date from
		 */
		void fromSystemTime(const SystemTimeStamp& systemTime);

		/**
		 * Converts a system time stamp into a date	
		 */
		SystemTimeStamp toSystemTime() const;

		EMonth	mMonth	= EMonth::Unknown;			///< Property: 'Month' the month of the year
		int		mDay	= 1;						///< Property: 'Day' the day of the year
		int		mYear	= 1970;						///< Property: 'Year' the year
	};


	//////////////////////////////////////////////////////////////////////////
	// Timestamp
	//////////////////////////////////////////////////////////////////////////

	/**
	* A serializable time stamp, represented as a 64 bit long value.
	* Use this object when serialization / de-serialization of a point in time is necessary.
	* When dealing with time at run-time use the default nap::DateTime object.
	* This is a relatively light weight object that can be both copy and move constructed or assigned.
	*/
	class NAPAPI TimeStamp final
	{
		RTTI_ENABLE()
	public:
		/**
		* Default Constructor
		*/
		TimeStamp() = default;

		/**
		 * Default Constructor
		 * @param timeStamp time since epoch as long
		 */
		TimeStamp(int64_t timeStamp) : mTimeStamp(timeStamp)	{ }

		/**
		 * Constructor based on given system time. Converts system time to a long that can be serialized.
		 * @param systemTime system time stamp to convert to serializable time stamp.
		 */
		TimeStamp(const SystemTimeStamp& systemTime);

		/**
		* Converts a system time stamp into a long that can be serialized
		* @param systemTime time stamp to convert
		*/
		void fromSystemTime(const SystemTimeStamp& systemTime);

		/**
		* @return the system time stamp based on stored internal time
		*/
		SystemTimeStamp toSystemTime() const;

		/**
		* @return if the timestamp managed by this object is valid, ie: has been set
		*/
		inline bool isValid() const { return mTimeStamp >= 0; }

		int64_t mTimeStamp = -1;		///< Property: 'Time' time since epoch stored as long
	};
}


//////////////////////////////////////////////////////////////////////////
// Hashes
//////////////////////////////////////////////////////////////////////////

namespace std
{
	template <>
	struct hash<nap::EDay>
	{
		size_t operator()(const nap::EDay& v) const
		{
            return std::hash<int>()(static_cast<int>(v));
		}
	};

	template <>
	struct hash<nap::EMonth>
	{
		size_t operator()(const nap::EMonth& v) const
		{
            return std::hash<int>()(static_cast<int>(v));
		}
	};
}
