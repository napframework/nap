/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// internal includes
#include "sequencetrack.h"
#include "sequencetracksegmentevent.h"

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <glm/glm.hpp>

namespace nap
{
    /**
     * Event track, holds a collection of SequenceTrackSegmentEvents
     */
    class NAPAPI SequenceTrackEvent : public SequenceTrack
    {
		RTTI_ENABLE(SequenceTrack)
    };
}
