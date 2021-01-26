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
	 * Simple read-only calendar, manages a set of calendar items.
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

		std::vector<nap::ResourcePtr<CalendarItem>> mItems;		///< Property: 'Items' all static calendar items
		Calendar::EUsage mUsage = EUsage::Static;				///< Property: 'Usage' how the calendar is used 

	private:
		std::unique_ptr<CalendarInstance> mInstance = nullptr;	///< Calendar runtime instance
		nap::Core& mCore;										///< NAP core
	};


	//////////////////////////////////////////////////////////////////////////
	// Calendar runtime instance
	//////////////////////////////////////////////////////////////////////////

	using CalendarItemList = std::vector<std::unique_ptr<CalendarItem>>;

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
		const std::string& getName() const				{ return mName; }

		/**
		 * @return absolute path on disk to calendar
		 */
		std::string getPath() const;

	protected:
		/**
		 * Initialize instance against resource
		 * @param calendar the resource to initialize against
		 * @param error contains the error if initialization fails
		 * @return if initialization succeeded
		 */
		bool init(const Calendar& resource, utility::ErrorState& error);

	private:
		CalendarItemList mItems;		///< All the items
		std::string mName = "";			///< Calendar name
		nap::Core& mCore;				///< NAP core
	};
}
