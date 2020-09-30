/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "sequence.h"

RTTI_BEGIN_CLASS(nap::Sequence)
	RTTI_PROPERTY("Sequence Tracks", &nap::Sequence::mTracks, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Duration", &nap::Sequence::mDuration, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
}
