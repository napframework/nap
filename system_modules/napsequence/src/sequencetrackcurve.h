/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// internal includes
#include "sequencetrack.h"
#include "sequencetracksegmentcurve.h"

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <glm/glm.hpp>

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    /**
     * Base class of all curve tracks
     */
    class NAPAPI BaseSequenceTrackCurve : public SequenceTrack
    {
		RTTI_ENABLE(SequenceTrack)
    };

    /**
     * Represents a track that holds a collection of segments, where T is the data represented by the track.
     * There are currently four supported SequenceTrackCurve types ( float, vec2, vec3, vec4 )
     */
    template<typename T>
    class SequenceTrackCurve : public BaseSequenceTrackCurve
    {
    RTTI_ENABLE(BaseSequenceTrackCurve)
    public:
        /**
         * initializes the curve segment and validates its data.
         * @param errorState contains any errors
         * @return returns true on successful initialization
         */
        virtual bool init(utility::ErrorState& errorState) override;

        T mMaximum = T(1); ///< Property: 'Maximum' maximum value of track
        T mMinimum = T(0); ///< Property: 'Minimum' minimum value of track
    };

    //////////////////////////////////////////////////////////////////////////
    // Definitions of all supported Sequence Curve Tracks
    //////////////////////////////////////////////////////////////////////////

    using SequenceTrackCurveFloat = SequenceTrackCurve<float>;
    using SequenceTrackCurveVec2 = SequenceTrackCurve<glm::vec2>;
    using SequenceTrackCurveVec3 = SequenceTrackCurve<glm::vec3>;
    using SequenceTrackCurveVec4 = SequenceTrackCurve<glm::vec4>;


    //////////////////////////////////////////////////////////////////////////
    // Template definitions
    //////////////////////////////////////////////////////////////////////////

    template<typename T>
    bool nap::SequenceTrackCurve<T>::init(utility::ErrorState& errorState)
    {
        // Initialize base class
        if(!SequenceTrack::init(errorState))
            return false;

        // Validate type of segment, needs to be of same type as this class!
        for(const auto &segment: mSegments)
        {
            if(!errorState.check(segment->get_type().template is_derived_from<SequenceTrackSegmentCurve<T>>(),
                                 "segment not derived from correct type, expected: %s, got: %s",
                                 RTTI_OF(SequenceTrackSegmentCurve<T>).get_name().data(),
                                 segment->get_type().get_name().data()))
            {
                return false;
            }
        }
        return true;
    }
}
