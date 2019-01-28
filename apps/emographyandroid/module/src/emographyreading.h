#pragma once

// External Includes
#include <nap/datetime.h>
#include <rtti/object.h>

namespace nap
{
	namespace emography
	{
		/**
		 * Base class for all readings, which is used to be able to retrieve the timestamp of a particular reading, without knowing the exact type.
		 */
		class ReadingBase : public rtti::Object
		{
			RTTI_ENABLE(rtti::Object)
		public:
			/**
			 * Default constructor
			 * Timestamp will be set to the time of creation
			 */
			ReadingBase() :
				mTimeStamp(getCurrentTime())
			{
			}

			/**
			 * Constructs a reading at a particular time.
			 * @param timestamp time associated with this reading
			 */
			ReadingBase(const TimeStamp& timeStamp) :
				mTimeStamp(timeStamp)
			{
			}

		public:
			TimeStamp mTimeStamp;        ///< Property: 'TimeStamp' Point in time at which this reading is taken
		};

		/**
		 * Represents a single state reading at a particular point in time.
		 * It combines both a timestamp and object associated with that time stamp.
		 * This object is serializable but not a resource.
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
			 * Constructs a reading of type T, a copy is made.
			 * @param object the object this reading holds
			 */
			Reading(const T& object) :
				ReadingBase(getCurrentTime()),
				mObject(object) 
			{ 
			}

			/**
			* Constructs a reading of type T at a particular time, a copy is made.
			* @param object the object this reading holds
			* @param timestamp time associated with this reading
			*/
			Reading(const T& object, const TimeStamp& timestamp) :
				ReadingBase(timestamp),
				mObject(object) 
			{
			}

			T mObject;					///< Property: 'Object' the object this reading manages
		};

		/**
		 * Represents a summary of multiple state readings over a range of time.
		 * It combines a timestamp (the start of the range) with an object containing the summarized state for that range of time.
		 * Next to that, it also keeps track of the number of seconds in the entire range represented by the summary, that actually had data in them
		 */
		class ReadingSummaryBase : public rtti::Object
		{
			RTTI_ENABLE(rtti::Object)
		public:
			/**
			 * Default constructor
			 * Timestamp will be set to the time of creation
			 */
			ReadingSummaryBase() = default;

			/**
			 * Constructor to convert a ReadingBase to a ReadingSummaryBase
			 * Timestamp will be set to the time of the ReadingBase
			 */
			ReadingSummaryBase(const ReadingBase& reading) :
				mTimeStamp(reading.mTimeStamp),
				mNumSecondsActive(0)
			{
			}

		public:
			TimeStamp	mTimeStamp;				///< Property: 'TimeStamp' the TimeStamp of the start of the range represented by this summary
			int			mNumSecondsActive = 0;	///< Property: 'NumSecondsActive' the number of seconds in the range that actually had data i nthem
		};

		/**
		 * Derived class of ReadingSummaryBase, which is used to hold the typed data.
		 */
		template<typename T>
		class ReadingSummary : public ReadingSummaryBase
		{
			RTTI_ENABLE(ReadingSummaryBase)
		public:
			/**
			 * Default constructor
			 * Timestamp will be set to the time of creation
			 */
			ReadingSummary() = default;


			/**
			 * Constructor to convert a ReadingBase to a ReadingSummaryBase. The ReadingBase passed in must be of type Reading<T>.
			 * This constructor is only here to be able to construct ReadingSummary<T> objects through RTTI; for regular construction, use the constructor taking a Reading<T>
			 * Timestamp will be set to the time of the ReadingBase
			 */
			ReadingSummary(const ReadingBase& readingBase) :
				ReadingSummaryBase(readingBase)
			{
				const Reading<T>* reading = rtti_cast<const Reading<T>>(&readingBase);
				assert(reading);

				mObject = reading->mObject;
			}

			/**
			 * Constructor to convert a Reading<T> to a ReadingSummary<T>
			 * Timestamp will be set to the time of the ReadingBase
			 */
			ReadingSummary(const Reading<T>& reading) :
				ReadingSummaryBase(reading),
				mObject(reading.mObject)
			{
			}

		public:
			T			mObject; ///< Property: 'Object' the object this reading summary manages
		};
	}
}


#define DEFINE_READING_RTTI(Type)																						\
	RTTI_BEGIN_CLASS(nap::emography::Reading<Type>)																		\
		RTTI_CONSTRUCTOR(const Type&, const nap::TimeStamp&)															\
		RTTI_CONSTRUCTOR(const Type&)																					\
		RTTI_PROPERTY("Object", &nap::emography::Reading<Type>::mObject, nap::rtti::EPropertyMetaData::Default)			\
	RTTI_END_STRUCT																										\
																														\
	RTTI_BEGIN_CLASS(nap::emography::ReadingSummary<Type>)																\
		RTTI_CONSTRUCTOR(const nap::emography::ReadingBase&)															\
		RTTI_PROPERTY("Object", &nap::emography::ReadingSummary<Type>::mObject, nap::rtti::EPropertyMetaData::Default)	\
	RTTI_END_STRUCT
