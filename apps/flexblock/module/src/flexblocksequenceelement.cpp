#include "flexblocksequenceelement.h"

RTTI_DEFINE_BASE(nap::FlexBlockSequenceElement)

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	FlexBlockSequenceElement::~FlexBlockSequenceElement()			{ }


	bool FlexBlockSequenceElement::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mKeyFrame != nullptr,
			"keyframe must be assigned %s", this->mID.c_str()))
			return false;

		/*
		if (!errorState.check(!(mType == FlexBlockSequenceElementType::Transition && mNextStance == nullptr),
			"next stance must be set if type is transition %s", this->mID.c_str()))
			return false;
			*/

		if (!errorState.check(mDuration >= 0.0f,
			"duration must be bigger or equal then 0 %s", this->mID.c_str()))
			return false;

		return true;
	}

	void FlexBlockSequenceElement::setStartTime(double startTime)
	{
		mStartTime = startTime;
	}

	bool FlexBlockSequenceElement::process(double time, std::vector<ParameterFloat*>& outInputs)
	{
		return (time >= mStartTime && time < mStartTime + mDuration);
	}
}