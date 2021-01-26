#pragma once

// Local Includes
#include "calendaritem.h"

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>

namespace nap
{
	class CalendarInstance;

	/**
	 * Default calendar, manages a set of calendar items.
	 */
	class NAPAPI Calendar : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		 * Creates the calendar instance
		 * @return if initialization succeeded
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		std::vector<nap::ResourcePtr<CalendarItem>> mItems;		 ///< Property: 'Items' all static calendar items
	};
}
