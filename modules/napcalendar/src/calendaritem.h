#pragma once

// External Includes
#include <nap/resource.h>
#include <glm/glm.hpp>
#include <nap/datetime.h>

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
		 * Simple serializable calendar time structure
		 */
		class Time final
		{
			RTTI_ENABLE()
		public:
			Time() = default;
			Time(int hour, int minute);
			int mHour	= 0;						///< Property: 'Hour' 0-23
			int mMinute	= 0;						///< Property: 'Minute' 0-59
		};

		/**
		 * @return if the time is valid
		 */
		bool init(utility::ErrorState& errorState) override;

		Time		mTime = { 0, 0 };				///< Property: 'Time' time of the event: hours (0-23) & minutes (0-59)
		Time		mDuration = { 0, 0 };			///< Property: 'Duration' length of event: hours (0-23) & minutes (0-59)
		std::string mTitle = "";					///< Property: 'Title' item title
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
		
		int mDay = 1;								///< Property: 'Day' day of the month (1-31)
	};


	/**
	 * Weekly recurring calendar item
	 */
	class NAPAPI WeeklyCalendarItem : public CalendarItem
	{
		RTTI_ENABLE(CalendarItem)
	public:
		/**
		 * @return if the day and time are valid
		 */
		bool init(utility::ErrorState& errorState) override;

		EDay mDay = EDay::Monday;					///< Property: 'Day' day of the week
	};


	/**
	 * Daily recurring calendar item
	 */
	class NAPAPI DailyCalendarItem : public CalendarItem
	{
		RTTI_ENABLE(CalendarItem)
	public:
		/**
		 * @return if the time is valid
		 */
		bool init(utility::ErrorState& errorState) override;
	};


	/**
	 * Unique calendar item
	 */
	class NAPAPI UniqueCalendarItem : public CalendarItem
	{
		RTTI_ENABLE(CalendarItem)
	public:
		/**
		 * @return if the date and time are valid
		 */
		bool init(utility::ErrorState& errorState) override;
		
		nap::Date mDate;						///< Property: 'Date' calendar date
	};
}
