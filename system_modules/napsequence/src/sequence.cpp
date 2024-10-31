/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "sequence.h"

RTTI_BEGIN_CLASS(nap::Sequence, "Sequence information")
        RTTI_PROPERTY("Sequence Tracks", &nap::Sequence::mTracks, nap::rtti::EPropertyMetaData::Embedded, "Tracks of the sequence")
        RTTI_PROPERTY("Sequence Markers", &nap::Sequence::mMarkers, nap::rtti::EPropertyMetaData::Embedded, "Markers in the sequence")
        RTTI_PROPERTY("Duration", &nap::Sequence::mDuration, nap::rtti::EPropertyMetaData::Default, "Duration of the sequence in seconds")
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{ }
