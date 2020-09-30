#pragma once

// internal includes
#include "sequencetracksegment.h"
#include "sequencetrack.h"

// external includes
#include <nap/resource.h>
#include <parameter.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Sequence holds all the information needed by the player to play a Sequence and link it to the right parameters
	 */
	class NAPAPI Sequence : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		std::vector<ResourcePtr<SequenceTrack>>	mTracks; ///< Property: 'Sequence Track' Vector holding resourceptrs to the SequenceTracks
		double mDuration = 0; ///< Property: 'Duration' the duration of the sequence
	};
}
