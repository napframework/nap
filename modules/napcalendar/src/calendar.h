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
	constexpr const char* calendarDirectory = "calendar";		///< Directory where all calendars are stored

	/**
	 * Simple calendar resource, manages a set of calendar items.
	 */
	class NAPAPI Calendar : public Resource
	{
		RTTI_ENABLE(Resource)
	public:

		// How this calendar is used
		enum struct EUsage : int
		{
			Static,					///< Calendar can't be updated after initialization, only contains 'Items'.
			Dynamic					///< Calendar can be loaded, updated and saved after initialization.
		};

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
		CalendarInstance& getInstance()							{ assert(mInstance != nullptr);  return *mInstance; }

		/**
		 * @return the calendar instance, only available after initialization
		 */
		const CalendarInstance& getInstance() const				{ assert(mInstance != nullptr);  return *mInstance; }

		bool mAllowFailure = true;								///< Property: 'AllowFailure' If initialization continues when loading a calendar from disk fails. In that case resource defaults are used.
		std::vector<nap::ResourcePtr<CalendarItem>> mItems;		///< Property: 'Items' all static calendar items

	private:
		std::unique_ptr<CalendarInstance> mInstance = nullptr;	///< Calendar runtime instance
		nap::Core& mCore;										///< NAP core
	};


	//////////////////////////////////////////////////////////////////////////
	// Calendar runtime instance
	//////////////////////////////////////////////////////////////////////////

	using CalendarItemList = std::vector<nap::ResourcePtr<CalendarItem>>;
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
		 * @return name of calendar
		 */
		const std::string& getName() const				{ assert(!mName.empty()); return mName; }

		/**
		 * @return absolute path on disk to calendar
		 */
		std::string getPath() const						{ assert(!mPath.empty()); return mPath; }

		/**
		 * Initialize the calendar using the given name and items.
		 * If a calendar with the given name is present on disk, it is loaded instead.
		 * @param name name of the calendar to create
		 * @param allowFailure if default items are created when loading from disk fails (file might be corrupt for example).
		 * @param defaultItems default set of items to create calendar with
		 * @param error contains the error if initialization fails
		 */
		bool init(const std::string& name, bool allowFailure, CalendarItemList defaultItems, utility::ErrorState& error);

	protected:
		/**
		 * Initialize instance against resource
		 * @param calendar the resource to initialize against
		 * @param error contains the error if initialization fails
		 * @return if initialization succeeded
		 */
		bool init(const Calendar& resource, utility::ErrorState& error);

	private:
		bool loadCalendar(utility::ErrorState& error);
		bool saveCalendar(utility::ErrorState& error);

		OwnedCalendarItemList mItems;		///< List of unique calendar items
		std::string mName;				///< Calendar name
		std::string mPath;				///< Path to calendar file on disk
		nap::Core& mCore;				///< NAP core
	};
}
