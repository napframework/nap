#pragma once

// Local Includes
#include "sequencetracksegment.h"
#include "sequenceevent.h"

// External Includes
#include <nap/event.h>

namespace nap
{
	/**
	 * 
	 */
	class SequenceTrackSegmentEvent : public SequenceTrackSegment
	{
		RTTI_ENABLE(SequenceTrackSegment)
	public:
		std::string mMessage;

		bool mDispatched = false;

		virtual const SequenceEventTypes::Types getEventType() const { return SequenceEventTypes::STRING; }
		
		virtual SequenceEventPtr createEvent() ;
	};
}
