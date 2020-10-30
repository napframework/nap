/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "sequencetracksegment.h"

RTTI_BEGIN_CLASS(nap::SequenceTrackSegment)
RTTI_PROPERTY("Start Time", &nap::SequenceTrackSegment::mStartTime, nap::rtti::EPropertyMetaData::Default)

RTTI_PROPERTY("Duration", &nap::SequenceTrackSegment::mDuration, nap::rtti::EPropertyMetaData::Default)

RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool SequenceTrackSegment::init(utility::ErrorState& errorState)
	{
		if (!Resource::init(errorState))
		{
			return false;
		}

		return true;
	}
}
