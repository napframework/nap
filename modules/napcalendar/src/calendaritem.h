/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/resource.h>
#include <nap/datetime.h>
#include <nap/numeric.h>

namespace nap
{
	/**
	 * Base class of all calendar items.
	 * Every item has a start time and duration. The title and description are optional.
	 * Calendar item resolution is minutes (by default).
	 *
	 * Derived classes must override active(). That method defines if the item
	 * currently 'occurs' based on the given timestamp.
	 */
	class NAPAPI CalendarItem : public Resource
	{
		RTTI_ENABLE(Resource)
	public:	

		/**
		 * Serializable calendar time structure.
		 * Can be copied and moved.
		 */
		struct NAPAPI Time
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
		struct NAPAPI Point
		{
			Point() = default;
			Point(Time time, Time duration);
			Time mTime;							///< Property: 'Time' time of the event: hours (0-23) & minutes (0-59)
			Time mDuration;						///< Property: 'Duration' length of event: hours (0-23) & minutes (0-59). Duration of 0 = never
			bool valid() const;  				///< Returns if time is valid
		};

		// Default Constructor
		CalendarItem() = default;

		// Item constructor
		CalendarItem(const Point& point, const std::string& title);

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
		virtual bool active(SystemTimeStamp timeStamp) const = 0;

		/**
		 * Updates item title
		 * @param title new item title
		 */
		void setTitle(const std::string& title);

		/**
		 * @return item title
		 */
		const std::string& getTitle() const;

		/**
		 * Updates item description
		 * @param description new item description
		 */
		void setDescription(const std::string& description);
	
		/**
		 * @return item description
		 */
		const std::string& getDescription() const;

		/**
		 * Updates item time and duration and ensures new settings are valid.
		 * @param point new time and duration
		 * @return if time and duration have been updated
		 */
		bool setPoint(const Point& point);

		/**
		 * @return item time and duration
		 */
		const Point& getPoint() const;

		/**
		 * Updates time and ensures it is valid.
		 * @param time new time
		 * @return if the time is updated
		 */
		bool setTime(const Time& time);

		/**
		 * @return item time
		 */
		const Time& getTime() const;

		/**
		 * Updates item duration.
		 * @param duration new duration
		 * @return if the duration is updated
		 */
		void setDuration(const Time& duration);

		/**
		 * @return item duration (hours, minutes)
		 */
		const Time& getDuration() const;

		std::string mTitle = "";			///< Property: 'Title' item title
		Point		mPoint;					///< Property; 'Point' point in time together with duration
		std::string	mDescription = "";		///< Property: 'Description' item description
	};


	//////////////////////////////////////////////////////////////////////////
	// Unique
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Unique calendar item. For example: a meeting or call.
	 */
	class NAPAPI UniqueCalendarItem : public CalendarItem
	{
		RTTI_ENABLE(CalendarItem)
	public:

		// Default constructor
		UniqueCalendarItem() = default;

		// Argument constructor
		UniqueCalendarItem(const CalendarItem::Point& point, const std::string& title, const Date& date);

		/**
		 * Initializes the unique calendar item.
		 * Checks if the date and time are valid.
		 * @return if the date and time are valid
		 */
		bool init(utility::ErrorState& errorState) override;
		
		/**
		 * Updates the calendar date, ensures the new date is valid
		 * @param date the new date
		 * @return if the date is updated
		 */
		bool setDate(const nap::Date& date);

		/**
		* @return the calendar date
		*/
		const nap::Date& getDate() const;

		/**
		 * @param timeStamp time to validate
		 * @return if the unique calender item is active.
		 */
		virtual bool active(SystemTimeStamp timeStamp) const override;

		nap::Date mDate;	///< Property: 'Date' calendar date
	};


	//////////////////////////////////////////////////////////////////////////
	// Yearly
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Yearly occurring calendar item. For example: Christmas, New Year etc.
	 */
	class NAPAPI YearlyCalendarItem : public CalendarItem
	{
		RTTI_ENABLE(CalendarItem)
	public:

		// Default constructor
		YearlyCalendarItem() = default;

		// Argument constructor
		YearlyCalendarItem(const CalendarItem::Point& point, const std::string& title, EMonth month, int day);

		/**
		* Initializes the yearly calendar item.
		* Checks if the is valid.
		* @return if the date is valid
		*/
		bool init(utility::ErrorState& errorState) override;

		/**
		* Updates the calendar date, checks if the new date is valid.
		* @param month new month
		* @param day new day in month
		* @return if the date is updated
		*/
		bool setDate(EMonth month, int day);

		/**
	 	 * @param timeStamp time to validate
		 * @return if the unique calender item is active.
		 */
		virtual bool active(SystemTimeStamp timeStamp) const override;

		int mDay = 1;						///< Property: 'Day' the day of the month (1-31)
		EMonth mMonth = EMonth::Unknown;	///< Property: 'Month' month of the year
	};


	//////////////////////////////////////////////////////////////////////////
	// Monthly
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Monthly recurring calendar item.
	 */
	class NAPAPI MonthlyCalendarItem : public CalendarItem
	{
		RTTI_ENABLE(CalendarItem)
	public:

		// Default constructor
		MonthlyCalendarItem() = default;

		// Argument constructor
		MonthlyCalendarItem(const CalendarItem::Point& point, const std::string& title, int day);

		/**
		 * @return if the day and time are valid
		 */
		bool init(utility::ErrorState& errorState) override;
		
		/**
		 * Sets the day of the month, ensures the day is in range (1-31).
		 * @param day the new day of the month (1-31)
		 * @return if the day has been updated
		 */
		bool setDay(int day);

		/**
		 * @return day of the month
		 */
		int getDay() const { return mDay; }

		/**
		 * @param timeStamp time to validate
		 * @return if the monthly calender item currently occurs.
		 */
		virtual bool active(SystemTimeStamp timeStamp) const override;
		
		int mDay = 1;	///< Property: 'Day' day of the month (1-31)
	};


	//////////////////////////////////////////////////////////////////////////
	// Weekly
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Weekly recurring calendar item. For example: Store opening hours.
	 */
	class NAPAPI WeeklyCalendarItem : public CalendarItem
	{
		RTTI_ENABLE(CalendarItem)
	public:

		// Default constructor
		WeeklyCalendarItem() = default;

		// Argument constructor
		WeeklyCalendarItem(const CalendarItem::Point& point, const std::string& title, EDay day);

		/**
		 * Initializes the weekly calendar item. 
		 * Checks if the day and time are valid.
		 * @return if the day and time are valid
		 */
		bool init(utility::ErrorState& errorState) override;

		/**
		 * Updates the day of the week
		 * @param day the new day of the week
		 * @return if the day of the week is updated
		 */
		bool setDay(EDay day);

		/**
		 * @return the day of the week
		 */
		EDay getDay() const;

		/**
		 * @param timeStamp time to validate
		 * @return if the weekly calender item currently occurs.
		 */
		virtual bool active(SystemTimeStamp timeStamp) const override;

		EDay mDay = EDay::Monday;	///< Property: 'Day' day of the week
	};


	//////////////////////////////////////////////////////////////////////////
	// Daily
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Daily recurring calendar item. For example: watering the plants or an alarm clock.
	 */
	class NAPAPI DailyCalendarItem : public CalendarItem
	{
		RTTI_ENABLE(CalendarItem)
	public: 
		// Default constructor
		DailyCalendarItem() = default;

		// Argument constructor
		DailyCalendarItem(const CalendarItem::Point& point, const std::string& title);

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
		virtual bool active(SystemTimeStamp timeStamp) const override;
	};
}
