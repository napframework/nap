/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequencetracksegmentaudio.h"

RTTI_BEGIN_CLASS(nap::SequenceTrackSegmentAudio, "Audio sequence track segment")
    RTTI_PROPERTY("BufferID", &nap::SequenceTrackSegmentAudio::mAudioBufferID, nap::rtti::EPropertyMetaData::Default, "Audio buffer identifier")
    RTTI_PROPERTY("Start Time In Segment", &nap::SequenceTrackSegmentAudio::mStartTimeInAudioSegment, nap::rtti::EPropertyMetaData::Default, "Audio segment start time")
RTTI_END_CLASS
