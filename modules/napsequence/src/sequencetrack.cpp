/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "sequencetrack.h"
#include "sequencetracksegmentcurve.h"

// external includes
#include <nap/resourceptr.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceTrack)
        RTTI_PROPERTY("Segments", &nap::SequenceTrack::mSegments, nap::rtti::EPropertyMetaData::Embedded)
        RTTI_PROPERTY("Output ID", &nap::SequenceTrack::mAssignedOutputID, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("Name", &nap::SequenceTrack::mName, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("Height", &nap::SequenceTrack::mTrackHeight, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
}