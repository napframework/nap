#include "flexblocksequencetransition.h"

// nap::FlexBlockSequenceElement run time class definition 
RTTI_BEGIN_CLASS(nap::FlexBlockSequenceTransition)
// Put additional properties here
RTTI_PROPERTY("Duration", &nap::FlexBlockSequenceElement::mDuration, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Curve", &nap::FlexBlockSequenceTransition::mCurve, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Inputs", &nap::FlexBlockSequenceTransition::mInputs, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{

	bool FlexBlockSequenceTransition::init(utility::ErrorState& errorState)
	{
		if (!FlexBlockSequenceElement::init(errorState))
			return false;

		if (!errorState.check(mInputs.size() > 0,
			"inputs must be at least bigger then zero %s", this->mID.c_str()))
			return false;

		if (mCurve == nullptr)
		{
			mProcessFunc = &FlexBlockSequenceTransition::processLinear;
		}
		else
		{
			mProcessFunc = &FlexBlockSequenceTransition::processWithCurve;
		}

		return true;
	}

	bool FlexBlockSequenceTransition::process(double time, std::vector<ParameterFloat*>& outInputs)
	{
		return (this->*mProcessFunc)(time, outInputs);
	}

	bool FlexBlockSequenceTransition::processWithCurve(double time, std::vector<ParameterFloat*>& outInputs)
	{
		if (!FlexBlockSequenceElement::process(time, outInputs))
			return false;

		float progress = (time - mStartTime) / mDuration;

		for (int i = 0; i < mInputs.size(); i++)
		{
			outInputs[i]->setValue(math::lerp<float>(mStartInputs[i], mInputs[i], mCurve->evaluate(progress)));
		}

		return true;
	}

	bool FlexBlockSequenceTransition::processLinear(double time, std::vector<ParameterFloat*>& outInputs)
	{
		if (!FlexBlockSequenceElement::process(time, outInputs))
			return false;

		float progress = (time - mStartTime) / mDuration;

		for (int i = 0; i < mInputs.size(); i++)
		{
			outInputs[i]->setValue(math::lerp<float>(mStartInputs[i], mInputs[i], progress));
		}

		return true;
	}

}