// local includes
#include "timelinetrack.h"

RTTI_BEGIN_CLASS(nap::TimelineTrack)
	RTTI_PROPERTY("Parameter", &nap::TimelineTrack::mParameterID, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("KeyFrames", &nap::TimelineTrack::mKeyFrames, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
}
