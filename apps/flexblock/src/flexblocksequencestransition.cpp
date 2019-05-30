#include "flexblocksequencetransition.h"

// nap::FlexBlockSequenceElement run time class definition 
RTTI_BEGIN_CLASS(nap::FlexBlockSequenceTransition)
// Put additional properties here
RTTI_PROPERTY("KeyFrame", &nap::FlexBlockSequenceElement::mKeyFrame, nap::rtti::EPropertyMetaData::Required)
RTTI_PROPERTY("Duration", &nap::FlexBlockSequenceElement::mDuration, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("NextKeyFrame", &nap::FlexBlockSequenceTransition::mNextKeyFrame, nap::rtti::EPropertyMetaData::Required)
RTTI_PROPERTY("Curve", &nap::FlexBlockSequenceTransition::mCurve, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{

	bool FlexBlockSequenceTransition::init(utility::ErrorState& errorState)
	{
		if (!FlexBlockSequenceElement::init(errorState))
			return false;

		if (!errorState.check(mKeyFrame != nullptr && mNextKeyFrame != nullptr,
			"next key frame must be assigned for transition element %s", this->mID.c_str()))
			return false;

		return true;
	}

	bool FlexBlockSequenceTransition::process(double time, std::vector<float>& outInputs)
	{
		if (!FlexBlockSequenceElement::process(time, outInputs))
			return false;

		float progress = ( time - mStartTime ) / mDuration;

		for (int i = 0; i < outInputs.size(); i++)
		{
			outInputs[i] = lerp(mKeyFrame->mInputs[i], mNextKeyFrame->mInputs[i], mCurve->evaluate(progress));
		}

		return true;
	}

	float FlexBlockSequenceTransition::lerp(float a, float b, float t)
	{
		return a + (b - a) * t;
	}
}