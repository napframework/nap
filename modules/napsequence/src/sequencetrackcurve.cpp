/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "sequencetrackcurve.h"
#include "sequencetracksegmentcurve.h"

// external includes
#include <nap/resourceptr.h>

RTTI_DEFINE_BASE(nap::BaseSequenceTrackCurve)

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
}