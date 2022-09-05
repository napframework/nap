/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// local includes
#include "sequencetracksegment.h"

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <glm/glm.hpp>

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    /**
     * Holds a collection of track segments
     */
    class NAPAPI SequenceTrack : public Resource
    {
        RTTI_ENABLE(Resource)
    public:
        /**
         * Deconstructor
         */
        virtual ~SequenceTrack()
        {
        };

        std::string mName; ///< Property : 'Name' Assigned name to this track
        std::string mAssignedOutputID;    ///< Property: 'Assigned Output ID' Assigned output to this track id
        std::vector<ResourcePtr<SequenceTrackSegment>>    mSegments;    ///< Property: 'Segments' Vector holding track segments
    };
}
