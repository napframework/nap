#include "flexblocksequencetransition.h"

// nap::FlexBlockSequenceElement run time class definition 
RTTI_BEGIN_CLASS(nap::FlexBlockSequenceTransition)
// Put additional properties here
RTTI_PROPERTY("Duration", &nap::FlexBlockSequenceElement::mDuration, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Curve", &nap::FlexBlockSequenceTransition::mCurve, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Parameters", &nap::FlexBlockSequenceTransition::mParameters, nap::rtti::EPropertyMetaData::Embedded)
RTTI_PROPERTY("Preset File", &nap::FlexBlockSequenceTransition::mPreset, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Use Preset", &nap::FlexBlockSequenceTransition::mUsePreset, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{

	bool FlexBlockSequenceTransition::init(utility::ErrorState& errorState)
	{
		if (!FlexBlockSequenceElement::init(errorState))
			return false;

		if (!errorState.check(mParameters.size() > 0,
			"parameters must be at least larger then zero %s", this->mID.c_str()))
			return false;

		for (int i = 0; i < mParameters.size(); i++)
		{
			
			if (mParameters[i]->get_type().is_derived_from<ParameterFloat>())
			{
				mFunctions.push_back(&FlexBlockSequenceTransition::process<ParameterNumeric<float>, float>);
			}
			else if (mParameters[i]->get_type().is_derived_from<ParameterInt>())
			{
				mFunctions.push_back(&FlexBlockSequenceTransition::process<ParameterNumeric<int>, int>);
			}
			else if (mParameters[i]->get_type().is_derived_from<ParameterVec2>())
			{
				mFunctions.push_back(&FlexBlockSequenceTransition::process<ParameterVec2, glm::vec2>);
			}
			else if (mParameters[i]->get_type().is_derived_from<ParameterVec3>())
			{
				mFunctions.push_back(&FlexBlockSequenceTransition::process<ParameterVec3, glm::vec3>);
			}
			else if (mParameters[i]->get_type().is_derived_from<ParameterIVec2>())
			{
				mFunctions.push_back(&FlexBlockSequenceTransition::process<ParameterIVec2, glm::ivec2>);
			}
			else if (mParameters[i]->get_type().is_derived_from<ParameterIVec3>())
			{
				mFunctions.push_back(&FlexBlockSequenceTransition::process<ParameterIVec3, glm::ivec3>);
			}
		}

		if (mCurve != nullptr)
			mEvaluateFunction = &FlexBlockSequenceTransition::evaluateCurve;
		else
			mEvaluateFunction = &FlexBlockSequenceTransition::evaluateLinear;

		return true;
	}

	bool FlexBlockSequenceTransition::process(double time, std::vector<Parameter*>& outParameters)
	{
		if (!FlexBlockSequenceElement::process(time, outParameters))
			return false;

		float progress = (time - mStartTime) / mDuration;

		for (int i = 0; i < outParameters.size(); i++)
		{
			(this->*mFunctions[i])(progress, mStartParameters[i].get(), outParameters[i]);
		}

		return true;
	}

	float FlexBlockSequenceTransition::evaluateCurve(float progress)
	{
		return mCurve->evaluate(progress);
	}


	float FlexBlockSequenceTransition::evaluateLinear(float progress)
	{
		return progress;
	}

	template<typename T1, class T2>
	inline void FlexBlockSequenceTransition::process(float progress, Parameter * in, Parameter * out)
	{
		T1* a = static_cast<T1*>(in);
		T1* b = static_cast<T1*>(out);

		b->setValue(math::lerp<T2>(a->mValue, b->mValue, (this->*mEvaluateFunction)(progress)));
	}
}