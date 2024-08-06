/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "sequencetracksegment.h"

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    class NAPAPI SequenceTrackSegmentDuration : public SequenceTrackSegment
    {
    RTTI_ENABLE(SequenceTrackSegment)
    public:
        // Default Constructor & destructor
        SequenceTrackSegmentDuration() = default;
        virtual ~SequenceTrackSegmentDuration() = default;

        double mDuration = 1.0; ///< Property: 'Duration' duration of the segment in the track
    };
}
