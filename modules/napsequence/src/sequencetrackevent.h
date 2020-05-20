#pragma once

// internal includes
#include "sequencetrack.h"
#include "sequencetracksegmentevent.h"

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <glm/glm.hpp>

namespace nap
{
	/**
	 * Event track, holds a collection of SequenceTrackSegmentEvents
	 */
	class NAPAPI SequenceTrackEvent : public SequenceTrack
	{
		RTTI_ENABLE(SequenceTrack)
	public:
         virtual ~SequenceTrackEvent(){};
	};
}
