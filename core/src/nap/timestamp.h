#pragma once

// External Includes
#include <rtti/rtti.h>
#include <utility/dllexport.h>
#include <utility/datetime.h>
#include <nap/numeric.h>

namespace nap
{
	/**
	 * A serializable time stamp that is stored internally as a long.
	 * Use this object when serialization / de-serialization of time is necessary.
	 * When dealing with time at run-time date use the default time related objects in utility.
	 * This is a relatively light weight object that can be both copy and move constructed or assigned.
	 */
	class NAPAPI TimeStamp final
	{	
		RTTI_ENABLE()
	public:
		/**
		 * Default Constructor	
		 */
		TimeStamp() = default;
		
		/**
		 * Constructor based on given system time. Converts system time to a long that can be serialized.
		 * @param systemTime system time stamp to convert to serializable time stamp.
		 */
		TimeStamp(utility::SystemTimeStamp systemTime);

		/**
		 * Converts a system time stamp into a long that can be serialized
		 * @param systemTime time stamp to convert
		 */
		void fromSystemTime(utility::SystemTimeStamp systemTime);

		/**
		 * @return the system time stamp based on stored internal time	
		 */
		utility::SystemTimeStamp toSystemTime() const;

		/**
		 * @return if the timestamp managed by this object is valid, ie: has been set	
		 */
		inline bool isValid()			{ return mTimeStamp >= 0; }

		long long mTimeStamp = -1;		///< Property: 'Time' time since epoch stored as long
	};
}
