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
