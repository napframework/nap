#include "flexblocksequence.h"

// nap::flexblockstancesequence run time class definition 
RTTI_BEGIN_CLASS(nap::flexblock::FlexblockSequence)
// Put additional properties here
RTTI_PROPERTY("Inputs", &nap::flexblock::FlexblockSequence::mInputs, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace flexblock
	{
		FlexblockSequence::~FlexblockSequence() { }


		bool FlexblockSequence::init(utility::ErrorState& errorState)
		{
			if (mStartParameters.size() == 0)
			{
				mStartParameters.clear();
				mOwnedParameters.clear();
				for (float input : mInputs)
				{
					mOwnedParameters.emplace_back(std::make_unique<ParameterFloat>());
					mOwnedParameters.back()->setValue(input);

					ResourcePtr<ParameterFloat> parameterFloatPtr = ResourcePtr<ParameterFloat>(mOwnedParameters.back().get());
					parameterFloatPtr->mID = mID + " GeneratedParameter " + std::to_string(mStartParameters.size());
					mStartParameters.emplace_back(parameterFloatPtr.get());
				}
			}

			if (!timeline::Sequence::init(errorState))
				return false;

			return true;
		}
	}
}