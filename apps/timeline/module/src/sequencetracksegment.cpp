// local includes
#include "sequencetracksegment.h"

RTTI_BEGIN_CLASS(nap::SequenceTrackSegment)
RTTI_PROPERTY("Start Time", &nap::SequenceTrackSegment::mStartTime, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Curve", &nap::SequenceTrackSegment::mCurve, nap::rtti::EPropertyMetaData::Embedded)
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

		if (!errorState.check(mCurve != nullptr, "Curve cannot be null!"))
		{
			return false;
		}

		mCurve->mPoints[mCurve->mPoints.size() - 1].mPos.mTime = mDuration;

		return true;
	}
}
