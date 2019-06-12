#include "flexblocksequencetransition.h"

/*
RTTI_BEGIN_CLASS(nap::flexblock::FlexblockSequenceTransition)
// Put additional properties here
RTTI_PROPERTY("Duration", &nap::timeline::SequenceElement::mDuration, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Curve", &nap::timeline::SequenceTransition::mCurve, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Preset File", &nap::timeline::SequenceElement::mPreset, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Use Preset", &nap::timeline::SequenceElement::mUsePreset, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS
*/

RTTI_BEGIN_CLASS(nap::flexblock::FlexblockSequenceTransition)
RTTI_PROPERTY("Inputs", &nap::flexblock::FlexblockSequenceTransition::mInputs, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS



//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace flexblock
	{
		bool FlexblockSequenceTransition::init(utility::ErrorState& errorState)
		{
			mEndParameters.clear();
			for (float input : mInputs)
			{
				mOwnedParameters.emplace_back(std::make_unique<ParameterFloat>());
				mOwnedParameters.back()->setValue(input);

				ResourcePtr<ParameterFloat> parameterFloatPtr = ResourcePtr<ParameterFloat>(mOwnedParameters.back().get());
				mEndParameters.emplace_back(static_cast<Parameter*>(parameterFloatPtr.get()));
			}

			if (!timeline::SequenceTransition::init(errorState))
				return false;

			return true;
		}
	}
}