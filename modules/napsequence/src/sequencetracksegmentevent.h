#pragma once

// Local Includes
#include "sequencetracksegment.h"
#include "sequenceevent.h"

// External Includes
#include <nap/event.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Base class for event segments
	 */
	class SequenceTrackSegmentEventBase : public SequenceTrackSegment
	{
		friend class SequencePlayerEventAdapter;

		RTTI_ENABLE(SequenceTrackSegment)
	public:
	private:
		/**
		 * creates an SequenceEventPtr.
		 * This method is called by SequencePlayerEventAdapter when a type of this event needs to be given to the main thread
		 */
		virtual SequenceEventPtr createEvent() = 0;
	};

	/**
	 * Event segments holding a value of type T
	 * @tparam T value type of event
	 */
	template <typename T>
	class SequenceTrackSegmentEvent : public SequenceTrackSegmentEventBase
	{
		RTTI_ENABLE(SequenceTrackSegmentEventBase)
	public:
		T mValue;
	private:
		virtual SequenceEventPtr createEvent() override ;
	};

	//////////////////////////////////////////////////////////////////////////
	// Definitions of all supported sequence track event segments
	//////////////////////////////////////////////////////////////////////////

	using SequenceTrackSegmentEventString = SequenceTrackSegmentEvent<std::string>;
	using SequenceTrackSegmentEventFloat = SequenceTrackSegmentEvent<float>;
	using SequenceTrackSegmentEventInt = SequenceTrackSegmentEvent<int>;
	using SequenceTrackSegmentEventVec2 = SequenceTrackSegmentEvent<glm::vec2>;
	using SequenceTrackSegmentEventVec3 = SequenceTrackSegmentEvent<glm::vec3>;

	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	SequenceEventPtr SequenceTrackSegmentEvent<T>::createEvent()
	{
		return std::make_unique<SequenceEvent<T>>(mValue);
	}
}
