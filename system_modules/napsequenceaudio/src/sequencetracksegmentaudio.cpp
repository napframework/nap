/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequencetracksegmentaudio.h"

RTTI_BEGIN_CLASS(nap::SequenceTrackSegmentAudio)
    RTTI_PROPERTY("BufferID", &nap::SequenceTrackSegmentAudio::mAudioBufferID, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Start Time In Segment", &nap::SequenceTrackSegmentAudio::mStartTimeInAudioSegment, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS