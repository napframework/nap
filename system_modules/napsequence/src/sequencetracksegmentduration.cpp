/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequencetracksegmentduration.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceTrackSegmentDuration, "Part of a track")
    RTTI_PROPERTY("Duration", &nap::SequenceTrackSegmentDuration::mDuration, nap::rtti::EPropertyMetaData::Default, "Duration of the segment in the track")
RTTI_END_CLASS