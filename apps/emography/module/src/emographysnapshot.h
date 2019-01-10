#pragma once

// Local Includes
#include "emographystate.h"

// External Includes
#include <utility/datetime.h>
#include <nap/timestamp.h>

namespace nap
{
	namespace emography
	{
		/**
		 * Represents a single emography state reading at a particular point in time.
		 * It combines both a stress related state and intensity value. 
		 * This object is serializable but not a resource.
		 * Because the object is relatively light weight it can be both copy and move constructed or assigned.
		 */
		class NAPAPI Snapshot final
		{
			RTTI_ENABLE()
		public:
			/**
			 * Constructs a snapshot with the given state and intensity at the current time
			 * @param state stress related state
			 * @param intensity intensity value
			 */
			Snapshot(EState state, float intensity);

			/**
			 * Constructs a snapshot at a particular time with the given intensity and state
			 * @param state stress related state
			 * @param intensity intensity value
			 * @param timestamp Point in time associated with this snapshot
			 */
			Snapshot(EState state, float intensity, utility::SystemTimeStamp stamp);

			/**
			 * Default constructor
			 */
			Snapshot() = default;

			/**
			 * @return if this snapshot is valid, ie: when intensity >= 0 and state != unknown
			 */
			inline bool isValid() const						
			{ 
				return mIntensity.isValid() && mState != EState::Unknown; 
			}

			Intensity					mIntensity;							///< Property: 'Intensity' stress intensity value
			EState						mState = EState::Unknown;			///< Property: 'State' stress related state
			TimeStamp					mTimeStamp;							///< Property: 'TimeStamp' Point in time at which this snapshot is taken
		};
	}
}
