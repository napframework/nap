#include "sequencetracksegmentnumeric.h"

RTTI_BEGIN_CLASS(nap::SequenceTrackSegmentNumeric)																						\
	RTTI_PROPERTY("Start Value", &nap::SequenceTrackSegmentNumeric::mStartValue, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("End Value", &nap::SequenceTrackSegmentNumeric::mEndValue, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Curve", &nap::SequenceTrackSegmentNumeric::mCurve, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS