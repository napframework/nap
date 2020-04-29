// local includes
#include "sequencetrack.h"
#include "sequencetracksegmentcurve.h"

// external includes
#include <nap/resourceptr.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceTrack)
	RTTI_PROPERTY("Segments",		&nap::SequenceTrack::mSegments, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Parameter ID",	&nap::SequenceTrack::mAssignedObjectIDs, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_DEFINE_BASE(nap::BaseSequenceTrackCurve)

RTTI_BEGIN_CLASS(nap::SequenceTrackEvent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::SequenceTrackCurveFloat)																						
	RTTI_PROPERTY("Minimum", &nap::SequenceTrackCurveFloat::mMinimum, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Maximum", &nap::SequenceTrackCurveFloat::mMaximum, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::SequenceTrackCurveVec2)
	RTTI_PROPERTY("Minimum", &nap::SequenceTrackCurveVec2::mMinimum, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Maximum", &nap::SequenceTrackCurveVec2::mMaximum, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::SequenceTrackCurveVec3)
	RTTI_PROPERTY("Minimum", &nap::SequenceTrackCurveVec3::mMinimum, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Maximum", &nap::SequenceTrackCurveVec3::mMaximum, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::SequenceTrackCurveVec4)
	RTTI_PROPERTY("Minimum", &nap::SequenceTrackCurveVec4::mMinimum, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Maximum", &nap::SequenceTrackCurveVec4::mMaximum, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	template<>
	SequenceTrackTypes::Types nap::SequenceTrackCurveFloat::getTrackType() const	{ return SequenceTrackTypes::FLOAT; }

	template<>
	SequenceTrackTypes::Types nap::SequenceTrackCurveVec2::getTrackType() const		{ return SequenceTrackTypes::VEC2; }

	template<>
	SequenceTrackTypes::Types nap::SequenceTrackCurveVec3::getTrackType() const		{ return SequenceTrackTypes::VEC3; }

	template<>
	SequenceTrackTypes::Types nap::SequenceTrackCurveVec4::getTrackType() const		{ return SequenceTrackTypes::VEC4; }
}