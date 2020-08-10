// local includes
#include "sequencetrack.h"
#include "sequencetracksegmentcurve.h"

// external includes
#include <nap/resourceptr.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceTrack)
	RTTI_PROPERTY("Segments",		&nap::SequenceTrack::mSegments, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Output ID",	&nap::SequenceTrack::mAssignedOutputID, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
}