/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// NAP Includes
#include <sequencetracksegment.h>

// External Includes
#include <nap/event.h>

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    /**
     * The SequenceTrackSegmentAudio contains a buffer id, pointer to the audio buffer id to use
     * Also, it contains a start time of the start position within the audio buffer
     */
    class SequenceTrackSegmentAudio : public SequenceTrackSegment
    {
        RTTI_ENABLE(SequenceTrackSegment)
    public:
        // buffer id
        std::string mAudioBufferID;

        // start time in audio buffer
        double mStartTimeInAudioSegment = 0.0;
    private:
    };
}
