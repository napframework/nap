#include "flexblocksequencestance.h"

// nap::FlexBlockSequenceElement run time class definition 
RTTI_BEGIN_CLASS(nap::FlexBlockSequenceStance)
// Put additional properties here
RTTI_PROPERTY("Duration", &nap::FlexBlockSequenceElement::mDuration, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("KeyFrame", &nap::FlexBlockSequenceElement::mKeyFrame, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{

	bool FlexBlockSequenceStance::init(utility::ErrorState& errorState)
	{
		if (!FlexBlockSequenceElement::init(errorState))
			return false;

		return true;
	}

	bool FlexBlockSequenceStance::process(double time, std::vector<float>& outInputs)
	{
		if (!FlexBlockSequenceElement::process(time, outInputs))
			return false;

		outInputs = mKeyFrame->mInputs;

		return true;
	}
}