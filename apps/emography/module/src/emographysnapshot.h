#pragma once

// Local Includes
#include "emographystress.h"

// External Includes
#include <nap/datetime.h>
#include <rtti/object.h>

namespace nap
{
	namespace emography
	{
		class ReadingBase : public rtti::Object
		{
			RTTI_ENABLE(rtti::Object)
		public:
			ReadingBase() :
				mTimeStamp(getCurrentTime())
			{
			}

			ReadingBase(const TimeStamp& timeStamp) :
				mTimeStamp(timeStamp)
			{
			}

		public:
			TimeStamp mTimeStamp;        ///< Property: 'TimeStamp' Point in time at which this snapshot is taken
		};

		/**
		 * Represents a single state reading at a particular point in time.
		 * It combines both a timestamp and object associated with that time stamp.
		 * This object is serializable but not a resource.
		 * Because the object is relatively light weight it can be both copy and move constructed or assigned.
		 */
		template<typename T>
		class Reading : public ReadingBase
		{
			RTTI_ENABLE(ReadingBase)
		public:
			/**
			 * Default constructor
			 * Timestamp will be set to the time of creation
			 */
			Reading() = default;

			/**
			 * Move Constructs a snapshot of type T, prevents unnecessary copy operations
			 * @param object the object this snapshot holds
			 */
			Reading(T&& object);

			/**
			 * Constructs a snapshot of type T, a copy is made.
			 * @param object the object this snapshot holds
			 */
			Reading(const T& object);

			/**
			 * Move Constructs a snapshot of type T at a particular time.
			 * @param object the object this snapshot holds
			 * @param timestamp time associated with this snapshot
			 */
			Reading(T&& object, TimeStamp&& timestamp);

			/**
			 * Constructs a snapshot of type T at a particular time, a copy is made.
			 * @param object the object this snapshot holds
			 * @param timestamp time associated with this snapshot
			 */
			Reading(const T& object, const TimeStamp& timestamp);

			T mObject;					///< Property: 'Object' the object this snapshot manages
		};

		class ReadingSummaryBase : public rtti::Object
		{
			RTTI_ENABLE(rtti::Object)
		public:
			ReadingSummaryBase() = default;

			ReadingSummaryBase(const ReadingBase& reading) :
				mTimeStamp(reading.mTimeStamp),
				mNumSecondsActive(1)
			{
			}

		public:
			TimeStamp	mTimeStamp;
			int			mNumSecondsActive = 0;
		};

		template<typename T>
		class ReadingSummary : public ReadingSummaryBase
		{
			RTTI_ENABLE(ReadingSummaryBase)
		public:
			ReadingSummary() = default;

			ReadingSummary(const ReadingBase& readingBase) :
				ReadingSummaryBase(readingBase)
			{
				const Reading<T>* reading = rtti_cast<const Reading<T>>(&readingBase);
				assert(reading);

				mObject = reading->mObject;
			}

			ReadingSummary(const Reading<T>& reading) :
				ReadingSummaryBase(reading),
				mObject(reading.mObject)
			{
			}

		public:
			T			mObject;
		};

		//////////////////////////////////////////////////////////////////////////
		// Implementations
		//////////////////////////////////////////////////////////////////////////

		// Possibly send towards android client after query
		using StressStateReading = Reading<EStressState>;
		using StressIntensityReading = Reading<StressIntensity>;
		using StressIntensityReadingSummary = ReadingSummary<StressIntensity>;


		//////////////////////////////////////////////////////////////////////////
		// Template definitions
		//////////////////////////////////////////////////////////////////////////

		template<typename T>
		nap::emography::Reading<T>::Reading(const T& object) :
			ReadingBase(getCurrentTime()),
			mObject(object)											{ }

		template<typename T>
		nap::emography::Reading<T>::Reading(T&& object) :
			ReadingBase(getCurrentTime()),
			mObject(std::move(object))								{ }

		template<typename T>
		nap::emography::Reading<T>::Reading(T&& object, TimeStamp&& timestamp) : 
			ReadingBase(std::move(timestamp)),
			mObject(std::move(object))								{ }

		template<typename T>
		nap::emography::Reading<T>::Reading(const T& object, const TimeStamp& timestamp) :
			ReadingBase(timestamp),
			mObject(object)											{ }
	}
}
