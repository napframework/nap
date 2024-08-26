#include "sequencetracksegmentcolor.h"

RTTI_BEGIN_ENUM(nap::SequenceTrackSegmentColor::EBlendMethod)
RTTI_ENUM_VALUE(nap::SequenceTrackSegmentColor::EBlendMethod::LINEAR, "Linear"),
RTTI_ENUM_VALUE(nap::SequenceTrackSegmentColor::EBlendMethod::OKLAB, "Oklab")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::SequenceTrackSegmentColor)
RTTI_PROPERTY("Color", &nap::SequenceTrackSegmentColor::mColor, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("BlendMethod", &nap::SequenceTrackSegmentColor::mBlendMethod, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS