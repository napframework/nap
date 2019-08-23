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
			if (mEndParameterResourcePtrs.size() != 
				mMotorInputs.size() // motor inputs
				+ 1 // slack
				+ mMotorOverrides.size() // motor overrides
				+ 2 // sinus frequency + sinus amplitude 
				)
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

				// motor override
				for (int i = 0; i < mMotorOverrides.size(); i++)
				{
					mOwnedParameters.emplace_back(std::make_unique<ParameterFloat>());
					mOwnedParameters.back()->setValue(mMotorOverrides[i]);

					ResourcePtr<ParameterFloat> parameterFloatPtr = ResourcePtr<ParameterFloat>(mOwnedParameters.back().get());
					parameterFloatPtr->mID = mID + " GeneratedParameter Override " + std::to_string(i);
					mEndParameters.emplace_back(static_cast<Parameter*>(parameterFloatPtr.get()));
					endParameterFloatPtrs.emplace_back(parameterFloatPtr);
				}

				// sinus frequency + amplitude
				{
					// frequency
					mOwnedParameters.emplace_back(std::make_unique<ParameterFloat>());
					mOwnedParameters.back()->setValue(mSinusFrequencey);

					ResourcePtr<ParameterFloat> frequencyFloatPtr = ResourcePtr<ParameterFloat>(mOwnedParameters.back().get());
					frequencyFloatPtr->mID = mID + " GeneratedParameter Sinus Frequency ";
					mEndParameters.emplace_back(static_cast<Parameter*>(frequencyFloatPtr.get()));
					endParameterFloatPtrs.emplace_back(frequencyFloatPtr);

					// amplitude
					mOwnedParameters.emplace_back(std::make_unique<ParameterFloat>());
					mOwnedParameters.back()->setValue(mSinusAmplitude);

					ResourcePtr<ParameterFloat> amplitudeFloatPtr = ResourcePtr<ParameterFloat>(mOwnedParameters.back().get());
					amplitudeFloatPtr->mID = mID + " GeneratedParameter Sinus Amplitude ";
					mEndParameters.emplace_back(static_cast<Parameter*>(amplitudeFloatPtr.get()));
					endParameterFloatPtrs.emplace_back(amplitudeFloatPtr);
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