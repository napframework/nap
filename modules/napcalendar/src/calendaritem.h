#pragma once

// External Includes
#include <nap/resource.h>
#include <glm/glm.hpp>
#include <nap/datetime.h>
#include <nap/numeric.h>

namespace nap
{
	/**
	 * Base calendar item
	 */
	class NAPAPI CalendarItem : public Resource
	{
		RTTI_ENABLE(Resource)
	public:	

		/**
		 * Serializable calendar time structure.
		 * Can be copied and moved.
		 */
		struct Time
		{
			Time() = default;
			Time(int hour, int minute);
			uint mHour	= 0;					///< Property: 'Hour' (0-23)
			uint mMinute	= 0;				///< Property: 'Minute' (0-59)
			nap::Minutes toMinutes() const;		///< Convert into minutes
		};

		/**
		 * Serializable calendar point in time structure.
		 * Represents point in time together with duration
		 */
		struct Point
		{
			Point() = default;
			Point(Time time, Time duration);
			Time mTime;							///< Property: 'Time' time of the event: hours (0-23) & minutes (0-59)
			Time mDuration;						///< Property: 'Duration' length of event: hours (0-23) & minutes (0-59). Duration of 0 = never
			bool valid() const;  				///< Returns if time is valid
		};

		/**
		 * Initializes the calendar item, always call this in derived classes.
		 * Ensures the given time is valid.
		 * @return If initialization succeeded.
		 */
		bool init(utility::ErrorState& errorState) override;

		/**
		 * Returns if the current item is active based  on the specified 
		 * 'Time', 'Duration' and other properties of this item.
		 * Must be implemented in derived classes.
		 * @param timeStamp time to validate, for example the current system time
		 * @return if the current item is active.
		 */
		virtual bool active(SystemTimeStamp timeStamp) = 0;

		std::string mTitle = "";					///< Property: 'Title' item title
		Point		mPoint;							///< Property; 'Point' point in time together with duration
		std::string	mDescription = "";				///< Property: 'Description' item description
	};


	/**
	 * Monthly recurring calendar item
	 */
	class NAPAPI MonthlyCalendarItem : public CalendarItem
	{
		RTTI_ENABLE(CalendarItem)
	public:
		/**
		 * @return if the day and time are valid
		 */
		bool init(utility::ErrorState& errorState) override;
		
		/**
		 * @param timeStamp time to validate
		 * @return if the monthly calender item currently occurs.
		 */
		virtual bool active(SystemTimeStamp timeStamp) override;
		int mDay = 1;						///< Property: 'Day' day of the month (1-31)
	};


	/**
	 * Weekly recurring calendar item
	 */
	class NAPAPI WeeklyCalendarItem : public CalendarItem
	{
		RTTI_ENABLE(CalendarItem)
	public:
		/**
		 * Initializes the weekly calendar item. 
		 * Checks if the day and time are valid.
		 * @return if the day and time are valid
		 */
		bool init(utility::ErrorState& errorState) override;

		/**
		 * @param timeStamp time to validate
		 * @return if the weekly calender item currently occurs.
		 */
		virtual bool active(SystemTimeStamp timeStamp) override;

		EDay mDay = EDay::Monday;	///< Property: 'Day' day of the week
	};


	/**
	 * Daily recurring calendar item
	 */
	class NAPAPI DailyCalendarItem : public CalendarItem
	{
		RTTI_ENABLE(CalendarItem)
	public: 
		/**
		 * Initializes the daily calendar item.
		 * Checks if the time is valid.
		 * @return if the time is valid
		 */
		bool init(utility::ErrorState& errorState) override;

		/**
		 * @param timeStamp time to validate
		 * @return if the daily calender item currently occurs.
		 */
		virtual bool active(SystemTimeStamp timeStamp) override;
	};


	/**
	 * Unique calendar item
	 */
	class NAPAPI UniqueCalendarItem : public CalendarItem
	{
		RTTI_ENABLE(CalendarItem)
	public:
		/**
		 * Initializes the unique calendar item.
		 * Checks if the date and time are valid.
		 * @return if the date and time are valid
		 */
		bool init(utility::ErrorState& errorState) override;
		
		/**
		 * @param timeStamp time to validate
		 * @return if the unique calender item currently occurs.
		 */
		virtual bool active(SystemTimeStamp timeStamp) override;

		nap::Date mDate;	///< Property: 'Date' calendar date
	};
}
