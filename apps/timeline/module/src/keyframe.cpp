// local includes
#include "keyframe.h"

RTTI_BEGIN_CLASS(nap::KeyFrame)
RTTI_PROPERTY("Name", &nap::KeyFrame::mName, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Value", &nap::KeyFrame::mValue, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Time", &nap::KeyFrame::mTime, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Next Curve", &nap::KeyFrame::mNextCurve, nap::rtti::EPropertyMetaData::Default)
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

		mCurve->mPoints[mCurve->mPoints.size() - 1].mPos.mTime = 1.0f;
		mCurve->mPoints[mCurve->mPoints.size() - 1].mPos.mValue = mValue;

		if (mNextCurve != nullptr)
		{
			mNextCurve->mPoints[0].mPos.mTime = 0.0f;
			mNextCurve->mPoints[0].mPos.mValue = mValue;
		}

		return true;
	}


	void KeyFrame::adjustValue(const float newValue)
	{
		mValue = newValue;

		mCurve->mPoints[mCurve->mPoints.size() - 1].mPos.mValue = mValue;
		if (mNextCurve != nullptr)
		{
			mNextCurve->mPoints[0].mPos.mValue = mValue;
		}
	}
}
