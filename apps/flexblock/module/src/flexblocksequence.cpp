#include "flexblocksequence.h"

// nap::flexblockstancesequence run time class definition 
RTTI_BEGIN_CLASS(nap::flexblock::FlexblockSequence)
// Put additional properties here
RTTI_PROPERTY("Motor Inputs", &nap::flexblock::FlexblockSequence::mMotorInputs, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace flexblock
	{
		FlexblockSequence::~FlexblockSequence() { }


		bool FlexblockSequence::init(utility::ErrorState& errorState)
		{
			mStartParameters.clear();
			for (float input : mMotorInputs)
			{
				mOwnedParameters.emplace_back(std::make_unique<ParameterFloat>());
				mOwnedParameters.back()->setValue(input);

				ResourcePtr<ParameterFloat> parameterFloatPtr = ResourcePtr<ParameterFloat>(mOwnedParameters.back().get());
				mStartParameters.emplace_back(parameterFloatPtr.get());
			}

			if (!timeline::Sequence::init(errorState))
				return false;

			return true;
		}
	}
}