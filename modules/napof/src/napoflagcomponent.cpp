#include "napoflagcomponent.h"
#include "ofUtils.h"
#include "utils/nofUtils.h"

namespace nap
{
	OFLagComponentBase::OFLagComponentBase()
	{
		mPreviousTime = ofGetElapsedTimef();
	}


	// Update call
	void OFLagComponentBase::onUpdate()
	{
		// Get time difference
		float current_time = ofGetElapsedTimef();
		float diff = current_time - mPreviousTime;
		mPreviousTime = current_time;

		// Perform damp
		UpdateTargetValue(diff);
	}


	//////////////////////////////////////////////////////////////////////////
	// Template implementations
	//////////////////////////////////////////////////////////////////////////
	
	template <> void OFLagComponent<float>::UpdateTargetValue(float inTimeDifference)
	{
		// Don't do anything when attribute isn't set
		if (mAttribute == nullptr)
			return;

		// Get current value
		mCurrentValue = gSmoothDamp(mCurrentValue, mTargetValue.getValue(), mCurrentVel, mSmoothTime.getValue(), mMaxSpeed.getValue(), inTimeDifference);

		// Set value
		mAttribute->getValueRef() = mCurrentValue;
	}


	template <> void OFLagComponent<ofVec3f>::UpdateTargetValue(float inTimeDifference)
	{
		// Don't do anything when attribute isn't set
		if (mAttribute == nullptr)
			return;

		mCurrentValue.x = gSmoothDamp(mCurrentValue.x, mTargetValue.getValue().x, mCurrentVel, mSmoothTime.getValue(), mMaxSpeed.getValue(), inTimeDifference);
		mCurrentValue.y = gSmoothDamp(mCurrentValue.y, mTargetValue.getValue().y, mCurrentVel, mSmoothTime.getValue(), mMaxSpeed.getValue(), inTimeDifference);
		mCurrentValue.z = gSmoothDamp(mCurrentValue.z, mTargetValue.getValue().z, mCurrentVel, mSmoothTime.getValue(), mMaxSpeed.getValue(), inTimeDifference);

		mAttribute->getValueRef() = mCurrentValue;
	}
}

RTTI_DEFINE(nap::OFLagComponentBase)
RTTI_DEFINE(nap::OFFloatLagComponent)
RTTI_DEFINE(nap::OFVec3LagComponent)
