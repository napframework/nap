// local includes
#include "timelinetrack.h"

RTTI_BEGIN_CLASS(nap::TimelineTrack)
	RTTI_PROPERTY("Parameter", &nap::TimelineTrack::mParameter, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("KeyFrames", &nap::TimelineTrack::mKeyFrames, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
}
