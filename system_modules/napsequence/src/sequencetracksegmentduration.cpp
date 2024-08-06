#include "sequencetracksegmentduration.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceTrackSegmentDuration, "Part of a track")
    RTTI_PROPERTY("Duration", &nap::SequenceTrackSegmentDuration::mDuration, nap::rtti::EPropertyMetaData::Default, "Duration of the segment in the track")
RTTI_END_CLASS