// local includes
#include "sequencetrack.h"

RTTI_BEGIN_CLASS(nap::SequenceTrack)
	RTTI_PROPERTY("Segments", &nap::SequenceTrack::mSegments, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
}
