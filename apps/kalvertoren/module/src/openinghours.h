#pragma once

// External Includes
#include <nap/resource.h>
#include <glm/vec2.hpp>

namespace nap
{
	/**
	 * Simple struct used to define an opening time
	 */
	class OpeningTime final
	{
	public:
		OpeningTime() = default;
		OpeningTime(int hour, int minute) : mHour(hour), mMinute(minute)	{ }

		int mHour = 0;				///< Property
		int mMinute = 0;			///< Property
	};

	/**
	 * Defines the opening hours for the kalvertoren shopping mall
	 */
	class OpeningHours : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~OpeningHours();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		// Property: All opening hours
		OpeningTime mMonday		= { 9,0  };
		OpeningTime mTuesday	= { 9,0  };
		OpeningTime mWednesday	= { 9,0  };
		OpeningTime mThursday	= { 9,0  };
		OpeningTime mFriday		= { 9,0  };
		OpeningTime mSaturday	= { 9,0  };
		OpeningTime mSunday		= { 11,0 };
	};
}
