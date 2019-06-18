//internal includes
#include "sequencetransition.h"

// external includes
#include <parametercolor.h>

// nap::FlexBlockSequenceElement run time class definition 
RTTI_BEGIN_CLASS(nap::timeline::SequenceTransition)
// Put additional properties here
RTTI_PROPERTY("Duration", &nap::timeline::SequenceElement::mDuration, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Curve", &nap::timeline::SequenceTransition::mCurve, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Parameters", &nap::timeline::SequenceTransition::mEndParameters, nap::rtti::EPropertyMetaData::Embedded)
RTTI_PROPERTY("Preset File", &nap::timeline::SequenceTransition::mPreset, nap::rtti::EPropertyMetaData::FileLink)
RTTI_PROPERTY("Use Preset", &nap::timeline::SequenceTransition::mUsePreset, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace timeline
	{
		bool SequenceTransition::init(utility::ErrorState& errorState)
		{
			if (!SequenceElement::init(errorState))
				return false;

			if (!errorState.check(mEndParameters.size() > 0,
				"parameters must be at least larger then zero %s", this->mID.c_str()))
				return false;

			for (int i = 0; i < mEndParameters.size(); i++)
			{
				if (mEndParameters[i]->get_type().is_derived_from<ParameterFloat>())
				{
					mFunctions.emplace_back(&SequenceTransition::process<ParameterFloat, float>);
				}
				else if (mEndParameters[i]->get_type().is_derived_from<ParameterInt>())
				{
					mFunctions.emplace_back(&SequenceTransition::process<ParameterInt, int>);
				}
				else if (mEndParameters[i]->get_type().is_derived_from<ParameterVec2>())
				{
					mFunctions.emplace_back(&SequenceTransition::process<ParameterVec2, glm::vec2>);
				}
				else if (mEndParameters[i]->get_type().is_derived_from<ParameterVec3>())
				{
					mFunctions.emplace_back(&SequenceTransition::process<ParameterVec3, glm::vec3>);
				}
				else if (mEndParameters[i]->get_type().is_derived_from<ParameterIVec2>())
				{
					mFunctions.emplace_back(&SequenceTransition::process<ParameterIVec2, glm::ivec2>);
				}
				else if (mEndParameters[i]->get_type().is_derived_from<ParameterIVec3>())
				{
					mFunctions.emplace_back(&SequenceTransition::process<ParameterIVec3, glm::ivec3>);
				}
				else if (mEndParameters[i]->get_type().is_derived_from<ParameterRGBColorFloat>())
				{
					mFunctions.emplace_back(&SequenceTransition::process<ParameterRGBColorFloat, RGBColorFloat>);
				}
				else if (mEndParameters[i]->get_type().is_derived_from<ParameterRGBAColorFloat>())
				{
					mFunctions.emplace_back(&SequenceTransition::process<ParameterRGBAColorFloat, RGBAColorFloat>);
				}
				else if (mEndParameters[i]->get_type().is_derived_from<ParameterRGBColor8>())
				{
					mFunctions.emplace_back(&SequenceTransition::process<ParameterRGBColor8, RGBColor8>);
				}
				else if (mEndParameters[i]->get_type().is_derived_from<ParameterRGBAColor8>())
				{
					mFunctions.emplace_back(&SequenceTransition::process<ParameterRGBAColor8, RGBAColor8>);
				}
				else
				{
					errorState.check(false, "No process function for type %s in %s",
						mEndParameters[i]->get_type().get_name(), mID.c_str());

					return false;
				}
			}

			if (mCurve != nullptr)
				mEvaluateFunction = &SequenceTransition::evaluateCurve;
			else
				mEvaluateFunction = &SequenceTransition::evaluateLinear;

			return true;
		}


		bool SequenceTransition::process(double time, std::vector<Parameter*>& outParameters)
		{
			if (!SequenceElement::process(time, outParameters))
				return false;

			float progress = (time - mStartTime) / mDuration;

			for (int i = 0; i < outParameters.size(); i++)
			{
				(this->*mFunctions[i])(progress, *mStartParameters[i], *mEndParameters[i], *outParameters[i]);
			}

			return true;
		}


		const float SequenceTransition::evaluateCurve(float progress)
		{
			return mCurve->evaluate(progress);
		}


		template<typename T1, typename T2>
		void SequenceTransition::process(float progress,
			const Parameter & inA,
			const Parameter & inB,
			Parameter & out)
		{
			static_cast<T1&>(out).setValue(
				math::lerp<T2>(
					static_cast<const T1&>(inA).mValue,
					static_cast<const T1&>(inB).mValue,
					(this->*mEvaluateFunction)(progress)));
		}
	}

	namespace math
	{
		template<>
		glm::ivec4 lerp<glm::ivec4>(const glm::ivec4& start, const glm::ivec4& end, float percent)
		{
			glm::ivec4 return_v;
			return_v.x = lerp<int>(start.x, end.x, percent);
			return_v.y = lerp<int>(start.y, end.y, percent);
			return_v.z = lerp<int>(start.z, end.z, percent);
			return_v.w = lerp<int>(start.w, end.w, percent);
			return return_v;
		}


		template<>
		glm::ivec3 lerp<glm::ivec3>(const glm::ivec3& start, const glm::ivec3& end, float percent)
		{
			glm::ivec3 return_v;
			return_v.x = lerp<int>(start.x, end.x, percent);
			return_v.y = lerp<int>(start.y, end.y, percent);
			return_v.z = lerp<int>(start.z, end.z, percent);
			return return_v;
		}


		template<>
		glm::ivec2 lerp<glm::ivec2>(const glm::ivec2& start, const glm::ivec2& end, float percent)
		{
			glm::ivec2 return_v;
			return_v.x = lerp<int>(start.x, end.x, percent);
			return_v.y = lerp<int>(start.y, end.y, percent);
			return return_v;
		}


		template<>
		int lerp<int>(const int& start, const int& end, float percent)
		{
			return glm::mix<int>(start, end, percent);
		}


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