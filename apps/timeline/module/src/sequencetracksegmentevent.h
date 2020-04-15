#pragma once

// Local Includes
#include "sequencetracksegment.h"

// External Includes

namespace nap
{
	namespace SequenceTrackSegmentEventTypes
	{
		enum Types
		{
			STRING,
			UNKOWN
		};
	}
	/**
	 * 
	 */
	class SequenceTrackSegmentEvent : public SequenceTrackSegment
	{
		RTTI_ENABLE(SequenceTrackSegment)
	public:
		std::string mMessage;

		bool mDispatched = false;

		virtual const SequenceTrackSegmentEventTypes::Types getEventType() const { return SequenceTrackSegmentEventTypes::STRING; }
	};
}
