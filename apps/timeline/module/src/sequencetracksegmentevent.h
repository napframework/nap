#pragma once

// Local Includes
#include "sequencetracksegment.h"
#include "sequenceevent.h"

// External Includes
#include <nap/event.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequencePlayerProcessorEvent;

	/**
	 * SequenceTrackSegmentEvent
	 * A SequenceTrackEvent that holds an event that can be trigger on a SequenceTrack
	 */
	class SequenceTrackSegmentEvent : public SequenceTrackSegment
	{
		friend class SequencePlayerProcessorEvent;

		RTTI_ENABLE(SequenceTrackSegment)
	public:
		std::string mMessage; ///< Property: 'Message' string of this message

		/**
		 * @return event type of this event
		 */
		virtual const SequenceEventTypes::Types getEventType() const { return SequenceEventTypes::STRING; }
		
	private:
		/**
		 * creates an SequenceEventPtr. 
		 * This method is called by SequencePlayerProcessorEvent when a type of this event needs to be given to the main thread
		 */
		virtual SequenceEventPtr createEvent() ;
	};
}
