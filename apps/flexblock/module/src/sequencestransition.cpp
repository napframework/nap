//internal includes
#include "sequencetransition.h"

// external includes
#include <parametercolor.h>

// nap::FlexBlockSequenceElement run time class definition 
RTTI_BEGIN_CLASS(nap::timeline::SequenceTransition)
// Put additional properties here
RTTI_PROPERTY("Duration", &nap::timeline::SequenceElement::mDuration, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Curves", &nap::timeline::SequenceTransition::mCurves, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Parameters", &nap::timeline::SequenceTransition::mEndParameterResourcePtrs, nap::rtti::EPropertyMetaData::Embedded)
RTTI_PROPERTY("Preset File", &nap::timeline::SequenceTransition::mPreset, nap::rtti::EPropertyMetaData::FileLink)
RTTI_PROPERTY("Use Preset", &nap::timeline::SequenceTransition::mUsePreset, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace timeline
	{
		std::map<rttr::type, TransitionProcess> SequenceTransition::mProcessFunctions =
		{
			{
				rttr::type::get<ParameterFloat>(),
				&SequenceTransition::process<ParameterFloat, float>
			},
			{
				rttr::type::get<ParameterInt>(),
				&SequenceTransition::process<ParameterInt, int>
			},
			{
				rttr::type::get<ParameterVec2>(),
				&SequenceTransition::process<ParameterVec2, glm::vec2>
			},
			{
				rttr::type::get<ParameterVec2>(),
				&SequenceTransition::process<ParameterVec3, glm::vec3>
			},
			{
				rttr::type::get<ParameterIVec2>(),
				&SequenceTransition::process<ParameterIVec2, glm::ivec2>
			},
			{
				rttr::type::get<ParameterRGBColorFloat>(),
				&SequenceTransition::process<ParameterRGBColorFloat, RGBColorFloat>
			},
			{
				rttr::type::get<ParameterRGBAColorFloat>(),
				&SequenceTransition::process<ParameterRGBAColorFloat, RGBAColorFloat>
			},
			{
				rttr::type::get<ParameterRGBColor8>(),
				&SequenceTransition::process<ParameterRGBColor8, RGBColor8>
			},
			{
				rttr::type::get<ParameterRGBAColor8>(),
				&SequenceTransition::process<ParameterRGBAColor8, RGBAColor8>
			}
		};

		bool SequenceTransition::init(utility::ErrorState& errorState)
		{
			if (!SequenceElement::init(errorState))
				return false;

			if (!errorState.check(mEndParameters.size() > 0,
				"parameters must be at least larger then zero %s", this->mID.c_str()))
				return false;

			mFunctions.clear();
			for (int i = 0; i < mEndParameters.size(); i++)
			{
				rttr::type type = mEndParameters[i]->get_type();
				if (mProcessFunctions.find(type) != mProcessFunctions.end())
				{
					mFunctions.emplace_back(mProcessFunctions[type]);
				}
				else
				{
					errorState.check(false, "No process function for type %s in %s",
						mEndParameters[i]->get_type().get_name().to_string().c_str(), mID.c_str());

					return false;
				}
			}

			for (int i = 0; i < mEndParameters.size(); i++)
			{
				bool needInsert = false;
				ResourcePtr<math::FloatFCurve> curve = nullptr;
				if (i < mCurves.size())
				{
					curve = mCurves[i];
				}
				else
				{
					needInsert = true;
				}
				
				if (curve == nullptr)
				{
					mOwnedCurves.emplace_back(std::make_unique<math::FloatFCurve>());

					curve = ResourcePtr<math::FloatFCurve>(mOwnedCurves.back().get());
					curve->mID = mID + "Generated_Curve" + std::to_string(i);
				}

				if (!needInsert)
				{
					mCurves[i] = curve;
				}
				else
				{
					mCurves.emplace_back(curve);
				}
			}

			return true;
		}


		bool SequenceTransition::process(double time, std::vector<Parameter*>& outParameters)
		{
			if (!SequenceElement::process(time, outParameters))
				return false;

			float progress = (time - mStartTime) / mDuration;

			for (int i = 0; i < outParameters.size(); i++)
			{
				(this->*mFunctions[i])(progress, *mStartParameters[i], *mEndParameters[i], *outParameters[i], i);
			}

			return true;
		}


		const float SequenceTransition::evaluate(float progress, const int curveIndex)
		{
			return mCurves[curveIndex]->evaluate(progress);
		}


		template<typename T1, typename T2>
		void SequenceTransition::process(float progress,
			const Parameter & inA,
			const Parameter & inB,
			Parameter & out,
			const int curveIndex)
		{
			static_cast<T1&>(out).setValue(
				math::lerp<T2>(
					static_cast<const T1&>(inA).mValue,
					static_cast<const T1&>(inB).mValue,
					this->evaluate(progress, curveIndex)));
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