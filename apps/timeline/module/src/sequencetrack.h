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

	/**
	 * SequenceTrack
	 * SequenceTrack holds a collection of track segments
	 */
	class NAPAPI SequenceTrack : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		std::vector<ResourcePtr<SequenceTrackSegment>>	mSegments; ///< Property: 'Segments' Vector holding track segments
	};
}
