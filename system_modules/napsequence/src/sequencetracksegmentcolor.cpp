#include "sequencetracksegmentcolor.h"

RTTI_BEGIN_ENUM(nap::SequenceTrackSegmentColor::EColorSpace)
RTTI_ENUM_VALUE(nap::SequenceTrackSegmentColor::EColorSpace::LINEAR, "Linear"),
RTTI_ENUM_VALUE(nap::SequenceTrackSegmentColor::EColorSpace::OKLAB, "Oklab")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::SequenceTrackSegmentColor)
RTTI_PROPERTY("Color", &nap::SequenceTrackSegmentColor::mColor, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("BlendMethod", &nap::SequenceTrackSegmentColor::mBlendMethod, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Curve", &nap::SequenceTrackSegmentColor::mCurve, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
RTTI_PROPERTY("CurveType", &nap::SequenceTrackSegmentColor::mCurveType, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS