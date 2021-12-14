/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// NAP includes
#include <sequencetrack.h>

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <glm/glm.hpp>

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    /**
     * The SequenceTrackAudio track type.
     * A SequenceTrackAudio contains SequenceTrackSegmentAudio type segments
     */
    class NAPAPI SequenceTrackAudio : public SequenceTrack
    {
        RTTI_ENABLE(SequenceTrack)
    public:
        /**
         * Deconstructor
         */
        virtual ~SequenceTrackAudio() = default;
    };
}
