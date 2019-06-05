#include "flexblocksequenceelement.h"

RTTI_DEFINE_BASE(nap::FlexBlockSequenceElement)

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	FlexBlockSequenceElement::~FlexBlockSequenceElement()			{ }


	bool FlexBlockSequenceElement::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mDuration >= 0.0f,
			"duration must be bigger or equal then 0 %s", this->mID.c_str()))
			return false;

		return true;
	}

	void FlexBlockSequenceElement::setStartInputs(const std::vector<float>& inputs)
	{
		mStartInputs = std::vector<float>(inputs.size());
		for (int i = 0; i < inputs.size(); i++)
		{
			mStartInputs[i] = inputs[i];
		}
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