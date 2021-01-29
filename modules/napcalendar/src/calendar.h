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
	// Calendar
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
	 * TODO: Make thread safe.
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
		 * Call saveCalendar() to write the calendar to disk.
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

		/**
		 * Creates, initializes and adds a new item of the given type T to the calendar.
		 * Item must be of type: nap::CalendarItem and is given a unique id.
		 * The item is managed by the calendar but can be changed through the pointer.
		 * This call is therefore not thread safe.
		 * Note that it is your responsibility to ensure that items, when edited, remain valid. 
		 *
		 * ~~~~~{.cpp}
		 *	auto* new_item = mCalendar->getInstance().addItem<UniqueCalendarItem>
		 *	(
		 *		CalendarItem::Point({ 12, 0 }, { 1, 0 }),
		 *		"Birthday Celebrations",
		 *		Date(2021, EMonth::January, 29)
		 *	);
		 * ~~~~~
		 *
		 * @param args arguments to construct the item with.
		 * @return the new calendar item, nullptr if the item can't be created or initialized
		 */
		template<typename T, typename... Args>
		T* addItem(Args&&... args);

		/**
		 * Remove an item based on id (not thread safe). 
		 * Note that handles are invalid after this call.
		 * @param id the unique id of the item to remove
		 * @return if the item was removed
		 */
		bool removeItem(const std::string& id);
		
		/**
		 * Remove an item based on the given handle (not thread safe).
		 * Note that given handle is set to null when removed.
		 * @param id the unique id of the item to remove
		 * @return if the item was removed
		 */
		bool removeItem(CalendarItem* item);

		/**
		 * Writes calendar to disk.
		 * @param error contains the error if writing fails
		 */
		bool save(utility::ErrorState& error);

		OwnedCalendarItemList mItems;		///< List of unique calendar items
		std::string mName;					///< Calendar name
		std::string mPath;					///< Path to calendar file on disk
		nap::Core& mCore;					///< NAP core

	private:
		/**
		 * Loads calendar from disk, automatically called on initialization
		 * @param error contains the error if loading fails.
		 */
		bool load(utility::ErrorState& error);
	};


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T, typename... Args>
	T* nap::CalendarInstance::addItem(Args&&... args)
	{
		// Construct using given (forwarded) arguments
		assert(RTTI_OF(T).is_derived_from(RTTI_OF(nap::CalendarItem)));
		std::unique_ptr<T> item = std::make_unique<T>(std::forward<Args>(args)...);

		// Initialize, on failure return and don't add as valid item
		item->mID = math::generateUUID();
		utility::ErrorState error;
		if (!item->init(error))
		{
			nap::Logger::error(error.toString());
			return nullptr;
		}

		T* item_ptr = item.get();
		mItems.emplace_back(std::move(item));
		return item_ptr;
	}
}
