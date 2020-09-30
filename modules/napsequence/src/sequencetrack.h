#pragma once

// local includes
#include "sequencetracksegment.h"

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <glm/glm.hpp>

namespace nap
{
	/**
	 * Holds a collection of track segments
	 */
	class NAPAPI SequenceTrack : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		 * Deconstructor
		 */
        virtual ~SequenceTrack(){};
        
		std::string mAssignedOutputID;	///< Property: 'Assigned Output ID' Assigned output to this track id
		std::vector<ResourcePtr<SequenceTrackSegment>>	mSegments;	///< Property: 'Segments' Vector holding track segments
	};
}
