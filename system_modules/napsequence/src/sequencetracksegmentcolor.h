/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "sequencetracksegmentduration.h"

// External Includes
#include <color.h>

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    class SequenceTrackSegmentColor : public SequenceTrackSegment
    {
    RTTI_ENABLE(SequenceTrackSegment)
    public:
        enum NAPAPI EColorSpace
        {
            LINEAR,
            OKLAB
        };

        // properties
        RGBAColorFloat mColor;
        SequenceTrackSegmentColor::EColorSpace mBlendMethod = SequenceTrackSegmentColor::EColorSpace::OKLAB;
        ResourcePtr<math::FCurve<float, float>> mCurve;
        math::ECurveInterp mCurveType = math::ECurveInterp::Linear;
    };
}
