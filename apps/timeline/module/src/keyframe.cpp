// local includes
#include "keyframe.h"

RTTI_BEGIN_CLASS(nap::KeyFrame)
RTTI_PROPERTY("Name", &nap::KeyFrame::mName, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Time", &nap::KeyFrame::mTime, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Curve", &nap::KeyFrame::mCurve, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool KeyFrame::init(utility::ErrorState& errorState)
	{
		if (!Resource::init(errorState))
		{
			return false;
		}

		if (!errorState.check(mCurve != nullptr, "Curve cannot be null!"))
		{
			return false;
		}

		return true;
	}
}
