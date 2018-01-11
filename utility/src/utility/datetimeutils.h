#pragma once

// Local includes
#include "utility/dllexport.h"

// External includes
#include <chrono>
#include <stdint.h>
#include <string>
#include <ctime>
#include <functional>

namespace nap
{
	namespace utility
	{
		// Typedefs
		using SystemClock = std::chrono::system_clock;							///< System clock, able to convert time points in to days, seconds etc.
		using HighResolutionClock = std::chrono::high_resolution_clock;			///< High resolution clock, works with the highest possible precision. Can't convert time points in to days, seconds etc.
		using Milliseconds = std::chrono::milliseconds;							///< Milliseconds type definition
		using NanoSeconds = std::chrono::nanoseconds;							///< Nanoseconds type definition
		using Seconds = std::chrono::seconds;									///< Seconds type definition
		using SystemTimeStamp = std::chrono::time_point<SystemClock>;			///< Point in time associated with the SystemClock
		using HighResTimeStamp = std::chrono::time_point<HighResolutionClock>;	///< Point in time associated with the HighResolutionClock

		// Forward declares
		class DateTime;

		/**
		 * @return the current time as a time stamp, this time is acquired using the system clock.
		 * this time can be converted in days, minutes etc using ctime related functions.
		 */
		extern SystemTimeStamp	getCurrentTime();

		/**
		 * @return a structure that contains the current date and time/
		 * Note that the time will be Local to this computer and includes daylight savings
		 */
		extern DateTime			getCurrentDateTime();

		/**
		 * Populates a DateTime structure that contains the current date and time
		 * Note that the time will be Local to this computer and includes daylight savings
		 * @param dateTime the time structure to populate with the current date and time

		 */
		extern void				getCurrentDateTime(DateTime& outDateTime);


		//////////////////////////////////////////////////////////////////////////
		// DateTime
		//////////////////////////////////////////////////////////////////////////

		/**
		 * Convenience enum that describes the day of the week
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
		extern std::string toString(EDay day);

		/**
		* Converts a string in to a day. This call is case-sensitive
		* @param string the name of the day, see EDay comments for string representation
		* @return the converted month, Unknown if not valid match is found.
		*/
		extern EDay toDay(const std::string& string);

		/**
		 *	Convenience enum that describes the month in a year
		 */
		enum class EMonth : int
		{
			January		= 0,		///< January
			February	= 1,		///< February
			March		= 2,		///< March
			April		= 3,		///< April
			May			= 4,		///< May
			June		= 5,		///< June
			July		= 6,		///< July
			August		= 7,		///< August
			September	= 8,		///< September
			October		= 9,		///< October
			November	= 10,		///< November
			December	= 11,		///< December
			Unknown		= -1		///< Unknown
		};
		
		/**
		* Converts a month in to a string
		* @param month the month to convert in to a string
		* @return the month as a string
		*/
		extern std::string toString(EMonth month);

		/**
		 * Converts a string in to a month. This call is case-sensitive
		 * @param string the name of the month, see EMonth comments for string representation
		 * @return the converted month, Unknown if not valid match is found.
		 */
		extern EMonth toMonth(const std::string& string);

		/**
		 * Contains the date and time extracted from the associated timestamp.
		 * This is a utility class that wraps a time stamp for easier readability and use
		 */
		class DateTime final
		{
		public:
			
			/**
			 * Specifies the way a timestamp is interpreted
			 */
			enum class ConversionMode : int
			{
				Local		= 0,		///< Local time, including possible daylight saving adjustment
				GMT			= 1			///< Greenwich Mean Time, excluding daylight saving adjustment
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
			SystemTimeStamp getTimeStamp() const													{ return mTimeStamp; }

		private:

			SystemTimeStamp		mTimeStamp = SystemClock::now();	///< The timestamp that contains all the timing information
			std::tm				mTimeStruct;						///< Extracted c style struct
			ConversionMode		mMode = ConversionMode::Local;		///< Current time conversion mode
		};


		//////////////////////////////////////////////////////////////////////////
		// Timer
		//////////////////////////////////////////////////////////////////////////

		/**
		* Keeps track of time from the moment the timer is started
		* This is a template Timer that can work with various chrono clocks
		* Use the utility classes SystemTimer and HighResolutionTimer to work with specific clocks
		* The template type T should be a specific type of chrono clock, ie: HighResolutionClock etc.
		* This timer is not threaded and doesn't work with callbacks
		*/
		template<typename Clock>
		class Timer
		{
		public:
			// Construction / Destruction
			Timer() = default;
			virtual ~Timer() = default;

			/**
			* Start the timer
			*/
			void start();

			/**
			* @return start time as point in time
			*/
			std::chrono::time_point<Clock> getStartTime() const;

			/**
			* Stop the timer, resets state
			*/
			void stop();

			/**
			 * Resets the timer and starts it again
			 */
			void reset();

			/**
			* @return the elapsed time in seconds as double
			*/
			double getElapsedTime() const;

			/**
			* @return the elapsed time in seconds as a float
			*/
			float getElapsedTimeFloat() const;

			/**
			* @return amount of processed ticks in milliseconds
			*/
			uint32_t getTicks() const;

		private:
			// Members
			std::chrono::time_point<Clock> mStart;
		};

		/**
		* Keeps track of time from the moment the timer is started
		* This timer uses the chrono SystemClock and should be sufficient for most time based operations
		* The timestamp associated with a SystemTimer can be converted to days, seconds, weeks etc.
		*/
        using SystemTimer = Timer<SystemClock>;


		/**
		* Keeps track of time from the moment the timer is started
		* This timer uses the chrono HighResolutionClock and should be used when extreme accuracy is important
		* The timestamp associated with a HighResolutionTime can not be converted to days, seconds, weeks etc.
		*/
        using HighResolutionTimer = Timer<HighResolutionClock>;
		

		//////////////////////////////////////////////////////////////////////////
		// Template Definitions
		//////////////////////////////////////////////////////////////////////////

		template<typename Clock>
		void Timer<Clock>::start()
		{
			mStart = Clock::now();
		}


		template<typename Clock>
		std::chrono::time_point<Clock> Timer<Clock>::getStartTime() const
		{
			return mStart;
		}


		// Stop timer
		template<typename Clock>
		void Timer<Clock>::stop()
		{
			mStart = std::chrono::time_point<Clock>(Milliseconds(0));
		}


		// Reset the timer
		template<typename Clock>
		void Timer<Clock>::reset()
		{
			start();
		}


		// Return number of ticks in milliseconds
		template<typename Clock>
		uint32_t Timer<Clock>::getTicks() const
		{
			auto elapsed = Clock::now() - mStart;
			return std::chrono::duration_cast<Milliseconds>(elapsed).count();
		}


		// Return elapsed time in seconds
		template<typename Clock>
		double Timer<Clock>::getElapsedTime() const
		{
			return std::chrono::duration<double>(Clock::now() - mStart).count();
		}


		// Elapsed time in seconds
		template<typename Clock>
		float Timer<Clock>::getElapsedTimeFloat() const
		{
			return std::chrono::duration<float>(Clock::now() - mStart).count();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// Hashes
//////////////////////////////////////////////////////////////////////////

namespace std
{
	template <>
	struct hash<nap::utility::EDay>
	{
		size_t operator()(const nap::utility::EDay& v) const
		{
            return std::hash<int>()(static_cast<int>(v));
		}
	};

	template <>
	struct hash<nap::utility::EMonth>
	{
		size_t operator()(const nap::utility::EMonth& v) const
		{
            return std::hash<int>()(static_cast<int>(v));
		}
	};
}
