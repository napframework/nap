// local includes
#include "flexblocksequencetransition.h"

RTTI_BEGIN_CLASS(nap::flexblock::FlexblockSequenceTransition)
RTTI_PROPERTY("Motor Inputs", &nap::flexblock::FlexblockSequenceTransition::mMotorInputs, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Slack", &nap::flexblock::FlexblockSequenceTransition::mSlack, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS



//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace flexblock
	{
		bool FlexblockSequenceTransition::init(utility::ErrorState& errorState)
		{
			if (mEndParameterResourcePtrs.size() != mMotorInputs.size() + 1)
			{
				//
				mEndParameters.clear();
				mOwnedParameters.clear();
				//mEndParameterResourcePtrs.clear();
				std::vector<ResourcePtr<Parameter>> endParameterFloatPtrs;
				for (int i = 0; i < mMotorInputs.size(); i++)
				{
					mOwnedParameters.emplace_back(std::make_unique<ParameterFloat>());
					
					float motorInput = mEndParameterResourcePtrs.size() > i ? static_cast<ParameterFloat*>(mEndParameterResourcePtrs[i].get())->mValue : mMotorInputs[i];
					mOwnedParameters.back()->setValue(motorInput);

					ResourcePtr<ParameterFloat> parameterFloatPtr = ResourcePtr<ParameterFloat>(mOwnedParameters.back().get());
					parameterFloatPtr->mID = mID + " GeneratedParameter " + std::to_string(i);
					mEndParameters.emplace_back(static_cast<Parameter*>(parameterFloatPtr.get()));
					endParameterFloatPtrs.emplace_back(parameterFloatPtr);
				}

				// slack
				{
					mOwnedParameters.emplace_back(std::make_unique<ParameterFloat>());
					mOwnedParameters.back()->setValue(mSlack);

					ResourcePtr<ParameterFloat> parameterFloatPtr = ResourcePtr<ParameterFloat>(mOwnedParameters.back().get());
					parameterFloatPtr->mID = mID + " GeneratedParameter Slack ";
					mEndParameters.emplace_back(static_cast<Parameter*>(parameterFloatPtr.get()));
					endParameterFloatPtrs.emplace_back(parameterFloatPtr);
				}

				mEndParameterResourcePtrs = endParameterFloatPtrs;
			}
			else
			{
				for (int i = 0; i < mEndParameterResourcePtrs.size(); i++)
				{
					mEndParameters.emplace_back(mEndParameterResourcePtrs[i].get());
				}
			}

			if (!timeline::SequenceTransition::init(errorState))
				return false;

			return true;
		}
	}
}