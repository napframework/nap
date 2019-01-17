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
		/**
		 * Represents a single state reading at a particular point in time.
		 * It combines both a timestamp and object associated with that time stamp.
		 * This object is serializable but not a resource.
		 * Because the object is relatively light weight it can be both copy and move constructed or assigned.
		 */
		template<typename T>
		class Reading : public rtti::Object
		{
			RTTI_ENABLE(rtti::Object)
		public:
			/**
			 * Default constructor
			 * Timestamp will be set to the time of creation
			 */
			Reading();

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

			TimeStamp mTimeStamp;		///< Property: 'TimeStamp' Point in time at which this snapshot is taken
			T mObject;					///< Property: 'Object' the object this snapshot manages
		};

		template<typename T>
		class ReadingSummary : public rtti::Object
		{
			RTTI_ENABLE(rtti::Object)
		public:
			ReadingSummary() = default;
			ReadingSummary(TimeStamp inStartTime, TimeStamp inEndTime, const T& inObject) :
				mStartTime(inStartTime),
				mEndTime(inEndTime),
				mObject(inObject)
			{
			}

			TimeStamp mStartTime;		///< Property: 'StartTime' Start point in time of the timerange this summary spans
			TimeStamp mEndTime;			///< Property: 'EndTime' End point in time of the timerange this summary spans
			T mObject;					///< Property: 'Object' the object this snapshot manages
		};


		//////////////////////////////////////////////////////////////////////////
		// Implementations
		//////////////////////////////////////////////////////////////////////////

		// Possibly send towards android client after query
		using StressStateReading = Reading<EStressState>;
		using StressStateReadingSummary = ReadingSummary<EStressState>;
		using StressIntensityReading = Reading<StressIntensity>;
		using StressIntensityReadingSummary = ReadingSummary<StressIntensity>;


		//////////////////////////////////////////////////////////////////////////
		// Template definitions
		//////////////////////////////////////////////////////////////////////////

		template<typename T>
		nap::emography::Reading<T>::Reading() :
			mTimeStamp(getCurrentTime())							{ }

		template<typename T>
		nap::emography::Reading<T>::Reading(const T& object) :
			mTimeStamp(getCurrentTime()),
			mObject(object)											{ }

		template<typename T>
		nap::emography::Reading<T>::Reading(T&& object) :
			mTimeStamp(getCurrentTime()),
			mObject(std::move(object))								{ }

		template<typename T>
		nap::emography::Reading<T>::Reading(T&& object, TimeStamp&& timestamp) : 
			mTimeStamp(std::move(timestamp)),
			mObject(std::move(object))								{ }

		template<typename T>
		nap::emography::Reading<T>::Reading(const T& object, const TimeStamp& timestamp) :
			mTimeStamp(timestamp),
			mObject(object)											{ }
	}
}
