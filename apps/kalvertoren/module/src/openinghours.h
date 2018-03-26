#pragma once

// External Includes
#include <nap/resource.h>
#include <glm/vec2.hpp>

namespace nap
{
	/**
	 * Simple struct used to define an opening time
	 */
	class OpeningTime
	{
	public:
		OpeningTime() = default;
		OpeningTime(int morningHour, int morningMinute, int eveningHour, int eveningMinute) :
			mMorningHour(morningHour),
			mMorningMinute(morningMinute),
			mEveningHour(eveningHour),
			mEveningMinute(eveningMinute)	{}

		int mMorningHour   = 0;		///< Property
		int mMorningMinute = 0;		///< Property
		int mEveningHour   = 0;		///< Property
		int mEveningMinute = 0;		///< Property
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
		OpeningTime mMonday		= { 9,0,19,0 };
		OpeningTime mTuesday	= { 9,0,19,0 };
		OpeningTime mWednesday	= { 9,0,19,0 };
		OpeningTime mThursday	= { 9,0,21,0 };
		OpeningTime mFriday		= { 9,0,19,0 };
		OpeningTime mSaturday	= { 9,0,19,0 };
		OpeningTime mSunday		= { 11,0,18,30 };
	};
}
