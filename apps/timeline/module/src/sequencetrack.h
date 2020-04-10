#pragma once

// internal includes
#include "sequencetracksegment.h"

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <parameternumeric.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	enum SequenceTrackTypes : int
	{
		NUMERIC,
		VEC3,
		UNKOWN
	};

	/**
	 * SequenceTrack
	 * SequenceTrack holds a collection of track segments
	 */
	class NAPAPI SequenceTrack : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		std::string mAssignedParameterID; ///< Property: 'Parameter ID' Assigned parameter id
		std::vector<ResourcePtr<SequenceTrackSegment>>	mSegments; ///< Property: 'Segments' Vector holding track segments
		int mTrackType = UNKOWN; ///< Property: 'Track Type' Type of segments this track holds
	
		const SequenceTrackTypes getTrackType() const { return static_cast<SequenceTrackTypes>(mTrackType); }
	};
}
