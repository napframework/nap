#pragma once

// Local Includes
#include "emographyreading.h"

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
		enum class EStressState : int
		{
			Under = 0,			///< Under stimulated
			Normal = 1,			///< Normally stimulated
			Over = 2,			///< Over stimulated
			Unknown = -1,		///< Unknown stimulated state

			Count = 3
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
			StressIntensity(float intensity) : mValue(intensity)	{ }

			/**
			 * @return if this is a valid intensity reading, ie: intensity value is >= 0
			 */
			inline bool isValid() const					{ return mValue >= 0.0f; }

			float mValue = -1.0f;			///< Property: "Value" the stress related intensity value
		};		

		using StressStateReading = Reading<EStressState>;

		using StressIntensityReading = Reading<StressIntensity>;
		using StressIntensityReadingSummary = ReadingSummary<StressIntensity>;

		/**
		 * The StressStateReadingSummary can summarize EStressState readings by keeping track of the number of samples of each state that's summarized in the summary object
	     */
		class NAPAPI StressStateReadingSummary : public ReadingSummaryBase
		{
			RTTI_ENABLE(ReadingSummaryBase)

		public:
			/**
			 * Default constructor
			 * Timestamp will be set to the time of creation
			 */
			StressStateReadingSummary() = default;

			/**
			 * Constructor to convert a ReadingBase to a ReadingSummaryBase. The ReadingBase passed in must be of type Reading<T>.
			 * This constructor is only here to be able to construct StressStateReadingSummary objects through RTTI
			 * Timestamp will be set to the time of the ReadingBase
			 */
			StressStateReadingSummary(const ReadingBase& readingBase);

			/**
			 * Add a StressStateReadingSummary to this summary
			 *
			 * @param other The StressStateReadingSummary to add
			 */
			void add(const StressStateReadingSummary& other);

			/**
			 * Get the count of the specified state (i.e. how many times that particular state was summarized in this summary object)
			 *
			 * @param inState The state to get the count for
			 * @return The count for the state
			 */
			int getCount(EStressState inState) const;

			/**
			 * Get the total count of all states in this summary
			 *
			 * @return The total count of all states in this summary
			 */
			int getTotalCount() const;

		public:
			int mUnderCount		= 0;	///< Property: "UnderCount" the number of samples of type EStressState::Under that have been summarized in this object
			int mNormalCount	= 0;	///< Property: "NormalCount" the number of samples of type EStressState::Normal that have been summarized in this object
			int mOverCount		= 0;	///< Property: "OverCount" the number of samples of type EStressState::Over that have been summarized in this object
		};
	}
}
