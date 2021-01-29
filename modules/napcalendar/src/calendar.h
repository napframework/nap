#pragma once

// Local Includes
#include "calendaritem.h"

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <nap/core.h>

namespace nap
{
	class CalendarInstance;
	using CalendarItemList = std::vector<nap::ResourcePtr<CalendarItem>>;
	constexpr const char* calendarDirectory = "calendar";		///< Directory where all calendars are stored

	/**
	 * Base class of all Calendars.
	 * Acts as an interface to the underlying calendar instance.
	 * Every derived class must create and return a calendar instance, but
	 * can provide it's own read-only interface in JSON.
	 */
	class NAPAPI ICalendar : public Resource
	{
		RTTI_ENABLE(Resource)

	public:
		// Constructor
		ICalendar(nap::Core& core);

		/**
		 * @return the calendar instance, only available after initialization
		 */
		virtual CalendarInstance& getInstance() = 0;

		/**
		 * @return the calendar instance, only available after initialization
		 */
		virtual const CalendarInstance& getInstance() const = 0;

	protected:
		nap::Core& mCore;					///< NAP core
	};


	//////////////////////////////////////////////////////////////////////////

	/**
	 * Simple calendar resource, manages a set of calendar items.
	 */
	class NAPAPI Calendar : public ICalendar
	{
		RTTI_ENABLE(ICalendar)
	public:
		// Constructor
		Calendar(nap::Core& core);

		// Destructor
		virtual ~Calendar();

		/**
		 * Creates and initializes the calendar instance
		 * @return if initialization succeeded
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the calendar instance, only available after initialization
		 */
		CalendarInstance& getInstance() override				{ assert(mInstance != nullptr);  return *mInstance; }

		/**
		 * @return the calendar instance, only available after initialization
		 */
		const CalendarInstance& getInstance() const override	{ assert(mInstance != nullptr);  return *mInstance; }

		CalendarItemList mItems;								///< Property: 'Items' default set of calendar items
		bool mAllowFailure = true;								///< Property: 'AllowFailure' If initialization continues when loading a calendar from disk fails. In that case resource defaults are used.

	private:
		std::unique_ptr<CalendarInstance> mInstance = nullptr;	///< Calendar runtime instance
	};


	//////////////////////////////////////////////////////////////////////////
	// Calendar runtime instance
	//////////////////////////////////////////////////////////////////////////

	using OwnedCalendarItemList = std::vector<std::unique_ptr<CalendarItem>>;

	/**
	 * Actual runtime version of a simple calendar, created by the calendar resource on initialization.
	 * Allows for inspection, creation, loading and saving of calendar items.
	 * TODO: Use SQLite database for faster item inspection and retrieval.
	 */
	class CalendarInstance final
	{
		friend class Calendar;
		RTTI_ENABLE()
	public:
		// Default constructor
		CalendarInstance(nap::Core& core);

		// Calendar can't be copied
		CalendarInstance(const CalendarInstance& rhs) = delete;
		CalendarInstance& operator=(const CalendarInstance& rhs) = delete;

		/**
		 * Initialize the calendar using the given name and items.
		 * If a calendar with the given name is present on disk, it is loaded instead.
		 * @param name name of the calendar to create
		 * @param allowFailure if default items are created when loading from disk fails (file might be corrupt for example).
		 * @param defaultItems default set of items to create calendar with
		 * @param error contains the error if initialization fails
		 */
		bool init(const std::string& name, bool allowFailure, CalendarItemList defaultItems, utility::ErrorState& error);

		/**
		 * @return name of calendar
		 */
		const std::string& getName() const							{ assert(!mName.empty()); return mName; }

		/**
		 * @return absolute path to calendar file on disk
		 */
		std::string getPath() const									{ assert(!mPath.empty()); return mPath; }

		/**
		 * @return all calendar items
		 */
		const OwnedCalendarItemList&  getItems() const				{ return mItems; }

	private:
		bool loadCalendar(utility::ErrorState& error);
		bool saveCalendar(utility::ErrorState& error);

		OwnedCalendarItemList mItems;		///< List of unique calendar items
		std::string mName;				///< Calendar name
		std::string mPath;				///< Path to calendar file on disk
		nap::Core& mCore;				///< NAP core
	};
}
