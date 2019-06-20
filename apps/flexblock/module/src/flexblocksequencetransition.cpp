// local includes
#include "flexblocksequencetransition.h"

RTTI_BEGIN_CLASS(nap::flexblock::FlexblockSequenceTransition)
RTTI_PROPERTY("Motor Inputs", &nap::flexblock::FlexblockSequenceTransition::mMotorInputs, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS



//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace flexblock
	{
		bool FlexblockSequenceTransition::init(utility::ErrorState& errorState)
		{
			mEndParameters.clear();
			for (float input : mMotorInputs)
			{
				mOwnedParameters.emplace_back(std::make_unique<ParameterFloat>());
				mOwnedParameters.back()->setValue(input);

				ResourcePtr<ParameterFloat> parameterFloatPtr = ResourcePtr<ParameterFloat>(mOwnedParameters.back().get());
				mEndParameterResourcePtrs.emplace_back(parameterFloatPtr);
			}

			if (!timeline::SequenceTransition::init(errorState))
				return false;

			return true;
		}
	}
}