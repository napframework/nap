#include "sequencetracksegmentevent.h"

RTTI_BEGIN_CLASS(nap::SequenceTrackSegmentEventFloat)
	RTTI_PROPERTY("Value", &nap::SequenceTrackSegmentEventFloat::mValue, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::SequenceTrackSegmentEventInt)
		RTTI_PROPERTY("Value", &nap::SequenceTrackSegmentEventInt::mValue, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::SequenceTrackSegmentEventString)
		RTTI_PROPERTY("Value", &nap::SequenceTrackSegmentEventString::mValue, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::SequenceTrackSegmentEventVec2)
		RTTI_PROPERTY("Value", &nap::SequenceTrackSegmentEventVec2::mValue, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::SequenceTrackSegmentEventVec3)
		RTTI_PROPERTY("Value", &nap::SequenceTrackSegmentEventVec3::mValue, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS