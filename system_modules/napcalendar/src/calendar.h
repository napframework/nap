/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "calendaritem.h"

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <nap/core.h>
#include <nap/logger.h>
#include <nap/signalslot.h>
#include <mathutils.h>

namespace nap
{
	class CalendarInstance;
	using CalendarItemList = std::vector<nap::ResourcePtr<CalendarItem>>;
	constexpr const char* calendarDirectory = "calendar";		///< Directory where all calendars are stored

	/**
	 * Base class of all Calendar types.
	 * Acts as an interface to the underlying calendar instance.
	 * Every derived class must create and return a calendar instance.
	 * Use the nap::CalendarComponent to receive a notification when an event starts and ends.
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
	 * Simple serializable calendar resource that manages a set of calendar items.
	 * Call getInstance() to inspect, create, load and save the calendar.
	 *
	 * The instance is created on initialization and is constructed using the
	 * set of 'Items' defined by this resource. If a calendar with the same ID
	 * exists on disk (saved previously) it is loaded instead.
	 *
	 * Use the nap::CalendarComponent to receive a notification when an event starts and ends.
	 * Note that this calendar is meant to be used with a 'limited' (1000x) set of events,
	 * as it doesn't make use of a database.
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
		bool mAllowFailure = true;								///< Property: 'AllowLoadFailure' If initialization continues when loading a calendar from disk fails. In that case resource defaults are used.

	private:
		std::unique_ptr<CalendarInstance> mInstance = nullptr;	///< Calendar runtime instance
	};


	//////////////////////////////////////////////////////////////////////////
	// Calendar runtime instance
	//////////////////////////////////////////////////////////////////////////

	using OwnedCalendarItemList = std::vector<std::unique_ptr<CalendarItem>>;

	/**
	 * Actual runtime version of a simple calendar, created by a nap::ICalendar resource on initialization.
	 * Allows for inspection, creation, loading and saving of calendar items.
	 * This model is: NOT THREAD SAFE. Don't edit, remove or add items on a different thread!
	 */
	class NAPAPI CalendarInstance final
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
		 * Returns all calendar items of type T.
		 * List is not cleared.
		 *
		 * ~~~~~{.cpp}
		 *	std::vector<WeeklyCalendarItem*> items;
		 *	mCalendar->getInstance().getItems(items);
		 * ~~~~~
		 *
		 * @param outItems all calendar items of type T
		 */
		template<typename T>
		void getItems(std::vector<T*>& outItems) const;

		/**
		 * Adds an item to this calendar.
		 * This call does not initialize the item for you.
		 * The new item must be initialized and valid.
		 * Ownership is transferred.
		 * @param item the item to add
		 */
		void addItem(std::unique_ptr<CalendarItem> item);

		/**
		 * Creates, initializes and adds a new item of the given type T to the calendar.
		 * Item must be of type: nap::CalendarItem and is given a unique id.
		 * The item is managed by the calendar.
		 *
		 * ~~~~~{.cpp}
		 *	UniqueCalendarItem* new_item = calendar.addItem<UniqueCalendarItem>
		 *	(
		 *		CalendarItem::Point({ 12, 0 }, { 1, 0 }),
		 *		"Birthday Celebrations",
		 *		Date(2021, EMonth::January, 29)
		 *	);
		 * ~~~~~
		 *
		 * @param args specific calendar item construction arguments
		 * @return the new calendar item, nullptr if the item can't be created or initialized
		 */
		template<typename T, typename... Args>
		T* addItem(Args&&... args);

		/**
		 * Remove an item based on id.
		 * Note that handles are invalid after this call.
		 * @param id the unique id of the item to remove
		 * @return if the item was removed
		 */
		bool removeItem(const std::string& id);

		/**
		 * Find a calendar item by ID
		 * @param id item unique ID
		 * @return calendar item if found, nullptr otherwise
		 */
		CalendarItem* findByID(const std::string& id);

		/**
		 * Find a calendar item of type T by ID
		 *
		 * ~~~~~{.cpp}
		 *	UniqueCalendarItem* item = calendar.findItem<UniqueCalendarItem>("9032091")
		 * ~~~~~
		 *
		 * @param id item unique ID
		 * @return calendar item of type T if found, nullptr otherwise
		 */
		template<typename T>
		T* findByID(const std::string& id)						{ return rtti_cast<T>(this->findByID(id)); }

		/**
		 * Find a calendar item by title, case sensitive.
		 * @param title item title
		 * @return calendar item if found, nullptr otherwise
		 */
		CalendarItem* findByTitle(const std::string& title);

		/**
		 * Find a calendar item of type T by title, case sensitive.
		 *
		 * ~~~~~{.cpp}
		 *	UniqueCalendarItem* item = calendar.findItem<UniqueCalendarItem>("birthday")
		 * ~~~~~
		 *
		 * @param title item title
		 * @return calendar item of type T if found, nullptr otherwise
		 */
		template<typename T>
		T* findByTitle(const std::string& title)		{ return rtti_cast<T>(this->findByTitle(title)); }

		/**
		 * Writes the calendar to disk.
		 * @param error contains the error if writing fails
		 */
		bool save(utility::ErrorState& error);

		Signal<const CalendarItem&> itemRemoved;	///< Called when an item is about to be removed
		Signal<const CalendarItem&> itemAdded;		///< Called when an item is added

		OwnedCalendarItemList mItems;		///< List of unique calendar items
		std::string mName;					///< Calendar name
		std::string mPath;					///< Path to calendar file on disk
		nap::Core& mCore;					///< NAP core

	private:
		/**
		 * Loads the calendar from disk, automatically called on initialization
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

	template<typename T>
	void nap::CalendarInstance::getItems(std::vector<T*>& outItems) const
	{
		outItems.clear();
		for (const auto& item : mItems)
		{
			T* c_item = rtti_cast<T>(item.get());
			if (c_item != nullptr)
			{
				outItems.emplace_back(c_item);
			}
		}
	}
}
