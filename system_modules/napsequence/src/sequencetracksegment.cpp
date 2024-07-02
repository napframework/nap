/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includeso
#include "sequencetracksegment.h"

RTTI_BEGIN_CLASS(nap::SequenceTrackSegment, "Part of a track")
        RTTI_PROPERTY("Label", &nap::SequenceTrackSegment::mLabel, nap::rtti::EPropertyMetaData::Default, "Segment name")
        RTTI_PROPERTY("Start Time", &nap::SequenceTrackSegment::mStartTime, nap::rtti::EPropertyMetaData::Default, "Segment start time")
        RTTI_PROPERTY("Duration", &nap::SequenceTrackSegment::mDuration, nap::rtti::EPropertyMetaData::Default, "Segment end time")
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
    bool SequenceTrackSegment::init(utility::ErrorState& errorState)
    {
		return Resource::init(errorState);
    }
}
