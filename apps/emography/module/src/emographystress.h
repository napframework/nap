#pragma once

// External Includes
#include <utility/dllexport.h>
#include <rtti/rtti.h>

namespace nap
{
	namespace emography
	{
		/**
		 * Represents the various stress related stimulation states
		 */
		enum class NAPAPI EStressState : int
		{
			Under	=  0,			///< Under stimulated
			Normal	=  1,			///< Normally stimulated
			Over	=  2,			///< Over stimulated
			Unknown = -1,			///< Unknown stimulated state
		};


		/**
		 * Represents an emography stress related intensity value.
		 * Simple struct like object that has only 1 field but is serializable.
		 * Because the object is relatively light weight it can be both copy and move constructed or assigned on the fly.
		 */
		class NAPAPI StressIntensity final
		{
			RTTI_ENABLE()
		public:

			/**
			 * Default constructor	
			 */
			StressIntensity() = default;

			/**
			* Constructor
			* @param intensity stress intensity value
			*/
			StressIntensity(float intensity);

			/**
			 * @return if this is a valid intensity reading, ie: intensity value is >= 0
			 */
			inline bool isValid() const					{ return mValue >= 0.0f; }

			float mValue = -1.0f;			///< Property: "Intensity" the stress related intensity value
		};


		/**
		 * Represents a single emography stress reading.
		 * This object combines both the stress intensity level and stress state into a single object.
		 * Together these can be used as an argument for a snapshot. 
		 * Because the object is relatively light weight it can be both copy and move constructed or assigned on the fly.
		 */
		class NAPAPI StressReading final
		{
			RTTI_ENABLE()
		public:
			/**
			 * Default Constructor
			 */
			StressReading() = default;

			/**
			 * Construct a reading with a given intensity and state.
			 * @param state stress related state
			 * @param intensity intensity value
			 */
			StressReading(EStressState state, float intensity);

			/**
			* @return if this snapshot is valid, ie: when intensity >= 0 and state != unknown
			*/
			inline bool isValid() const
			{
				return mIntensity.isValid() && mState != EStressState::Unknown;
			}

			StressIntensity				mIntensity;							///< Property: 'Intensity' stress intensity value
			EStressState				mState = EStressState::Unknown;		///< Property: 'State' stress related state
		};
	}
}
