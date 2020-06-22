#pragma once

// local includes
#include <sequencetrack.h>

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <glm/glm.hpp>



namespace nap
{
	class NAPAPI SequenceTrackAudio : public SequenceTrack
	{
		RTTI_ENABLE(SequenceTrack)
	public:
		virtual ~SequenceTrackAudio() {};
	};
}
