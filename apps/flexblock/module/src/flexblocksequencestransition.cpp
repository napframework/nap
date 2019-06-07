#include "flexblocksequencetransition.h"

#include <parametercolor.h>

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
				mFunctions.push_back(&FlexBlockSequenceTransition::process<ParameterFloat, float>);
			}
			else if (mParameters[i]->get_type().is_derived_from<ParameterInt>())
			{
				mFunctions.push_back(&FlexBlockSequenceTransition::process<ParameterInt, int>);
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
			else if (mParameters[i]->get_type().is_derived_from<ParameterRGBColorFloat>())
			{
				mFunctions.push_back(&FlexBlockSequenceTransition::process<ParameterRGBColorFloat, RGBColorFloat>);
			}
			else if (mParameters[i]->get_type().is_derived_from<ParameterRGBAColorFloat>())
			{
				mFunctions.push_back(&FlexBlockSequenceTransition::process<ParameterRGBAColorFloat, RGBAColorFloat>);
			}
			else if (mParameters[i]->get_type().is_derived_from<ParameterRGBColor8>())
			{
				mFunctions.push_back(&FlexBlockSequenceTransition::process<ParameterRGBColor8, RGBColor8>);
			}
			else if (mParameters[i]->get_type().is_derived_from<ParameterRGBAColor8>())
			{
				mFunctions.push_back(&FlexBlockSequenceTransition::process<ParameterRGBAColor8, RGBAColor8>);
			}
			else
			{
				errorState.check(false, "No process function for type %s in %s",
					mParameters[i]->get_type().get_name(), mID.c_str());

				return false;
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
			(this->*mFunctions[i])(progress, mStartParameters[i], mParameters[i], outParameters[i]);
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
	void FlexBlockSequenceTransition::process(float progress, 
		Parameter * inA,
		Parameter * inB,
		Parameter * out)
	{
		static_cast<T1*>(out)->setValue(
			math::lerp<T2>(
				static_cast<T1*>(inA)->mValue,
				static_cast<T1*>(inB)->mValue,
				(this->*mEvaluateFunction)(progress)));
	}

	namespace math
	{
		template<>
		nap::RGBColorFloat lerp<RGBColorFloat>(const nap::RGBColorFloat& start, const nap::RGBColorFloat& end, float percent)
		{
			nap::RGBColorFloat r;
			r.setRed(lerp<float>(start.getRed(), end.getRed(), percent));
			r.setGreen(lerp<float>(start.getGreen(), end.getGreen(), percent));
			r.setBlue(lerp<float>(start.getBlue(), end.getBlue(), percent));
			return r;
		}

		template<>
		nap::RGBAColorFloat lerp<RGBAColorFloat>(const nap::RGBAColorFloat& start, const nap::RGBAColorFloat& end, float percent)
		{
			nap::RGBAColorFloat r;
			r.setRed(lerp<float>(start.getRed(), end.getRed(), percent));
			r.setGreen(lerp<float>(start.getGreen(), end.getGreen(), percent));
			r.setBlue(lerp<float>(start.getBlue(), end.getBlue(), percent));
			r.setAlpha(lerp<float>(start.getAlpha(), end.getAlpha(), percent));
			return r;
		}

		template<>
		nap::RGBColor8 lerp<RGBColor8>(const nap::RGBColor8& start, const nap::RGBColor8& end, float percent)
		{
			nap::RGBColor8 r;
			r.setRed(lerp<int>(start.getRed(), end.getRed(), percent));
			r.setGreen(lerp<int>(start.getGreen(), end.getGreen(), percent));
			r.setBlue(lerp<int>(start.getBlue(), end.getBlue(), percent));
			return r;
		}

		template<>
		nap::RGBAColor8 lerp<RGBAColor8>(const nap::RGBAColor8& start, const nap::RGBAColor8& end, float percent)
		{
			nap::RGBAColor8 r;
			r.setRed(lerp<int>(start.getRed(), end.getRed(), percent));
			r.setGreen(lerp<int>(start.getGreen(), end.getGreen(), percent));
			r.setBlue(lerp<int>(start.getBlue(), end.getBlue(), percent));
			r.setAlpha(lerp<int>(start.getAlpha(), end.getAlpha(), percent));
			return r;
		}
	}
}