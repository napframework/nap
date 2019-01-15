#pragma once

// Local Includes
#include "emographystress.h"

// External Includes
#include <nap/datetime.h>

namespace nap
{
	namespace emography
	{
		/**
		 * Base class of a snapshot that can be serialized.
		 * Every snapshot can only be constructed using copy / move enabled objects, ie: no resources.
		 * A snapshot represents a state at a particular point in time.
		 */
		class NAPAPI BaseSnapshot
		{
			RTTI_ENABLE()
		public:
			/**
			 * Default constructor.
			 * Timestamp will be set to the time of creation
			 */
			BaseSnapshot();

			/**
			 * Default destructor
			 */
			virtual ~BaseSnapshot() = default;

			/**
			 * Copy constructor
			 * Snapshot taken at a particular point in time
			 * @param timestamp time associated with this snapshot
			 */
			BaseSnapshot(const TimeStamp& timestamp);

			/**
			 * Move constructor
			 * Snapshot taken at a particular point in time
			 * @param timestamp time associated with this snapshot
			 */
			BaseSnapshot(TimeStamp&& timestamp);

			TimeStamp mTimeStamp;							///< Property: 'TimeStamp' Point in time at which this snapshot is taken
		};


		/**
		 * Represents a single state reading at a particular point in time.
		 * It combines both a timestamp and object associated with that time stamp.
		 * This object is serializable but not a resource.
		 * Because the object is relatively light weight it can be both copy and move constructed or assigned.
		 */
		template<typename T>
		class Snapshot : public BaseSnapshot
		{
			RTTI_ENABLE(BaseSnapshot)
		public:
			/**
			 * Default constructor
			 * Timestamp will be set to the time of creation
			 */
			Snapshot();

			/**
			 * Move Constructs a snapshot of type T, prevents unnecessary copy operations
			 * @param object the object this snapshot holds
			 */
			Snapshot(T&& object);

			/**
			 * Constructs a snapshot of type T, a copy is made.
			 * @param object the object this snapshot holds
			 */
			Snapshot(const T& object);

			/**
			 * Move Constructs a snapshot of type T at a particular time.
			 * @param object the object this snapshot holds
			 * @param timestamp time associated with this snapshot
			 */
			Snapshot(T&& object, TimeStamp&& timestamp);

			/**
			 * Constructs a snapshot of type T at a particular time, a copy is made.
			 * @param object the object this snapshot holds
			 * @param timestamp time associated with this snapshot
			 */
			Snapshot(const T& object, const TimeStamp& timestamp);

			T mObject;		///< Property: 'Object' the object this snapshot manages
		};


		//////////////////////////////////////////////////////////////////////////
		// Implementations
		//////////////////////////////////////////////////////////////////////////

		// Possibly send towards android client after query
		using StressSnapshot = Snapshot<StressReading>;


		//////////////////////////////////////////////////////////////////////////
		// Template definitions
		//////////////////////////////////////////////////////////////////////////

		template<typename T>
		nap::emography::Snapshot<T>::Snapshot() : BaseSnapshot()	{ }

		template<typename T>
		nap::emography::Snapshot<T>::Snapshot(const T& object) : BaseSnapshot(),	
			mObject(object)											{ }

		template<typename T>
		nap::emography::Snapshot<T>::Snapshot(T&& object) : BaseSnapshot(),
			mObject(std::move(object))								{ }

		template<typename T>
		nap::emography::Snapshot<T>::Snapshot(T&& object, TimeStamp&& timestamp) : 
			BaseSnapshot(std::move(timestamp)),
			mObject(std::move(object))								{ }

		template<typename T>
		nap::emography::Snapshot<T>::Snapshot(const T& object, const TimeStamp& timestamp) :
			BaseSnapshot(timestamp),
			mObject(object)											{ }
	}
}
