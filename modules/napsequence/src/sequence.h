/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// internal includes
#include "sequencetracksegment.h"
#include "sequencetrack.h"
#include "sequencemarker.h"

// external includes
#include <nap/resource.h>

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    /**
     * Sequence holds all the information needed by the player to play a Sequence and link it to the right parameters
     */
    class NAPAPI Sequence : public Resource
    {
        RTTI_ENABLE(Resource)
    public:
        std::vector<ResourcePtr<SequenceTrack>> mTracks; ///< Property: 'Sequence Tracks' Vector holding resourceptrs to the SequenceTracks
        std::vector<ResourcePtr<SequenceMarker>> mMarkers; ///< Property: 'Sequence Markers' Vector holding resourceptrs to the SequenceMarkers
        double mDuration = 0; ///< Property: 'Duration' the duration of the sequence
    };
}
