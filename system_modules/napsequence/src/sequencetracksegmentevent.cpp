/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequencetracksegmentevent.h"

RTTI_DEFINE_BASE(nap::SequenceTrackSegmentEventBase, "Event track segment")

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

RTTI_BEGIN_CLASS(nap::SequenceTrackSegmentEventVec4)
		RTTI_PROPERTY("Value", &nap::SequenceTrackSegmentEventVec4::mValue, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS
