#pragma once

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <fcurve.h>
#include <math.h>
#include <parameternumeric.h>
#include <parametersimple.h>
#include <parametervec.h>
#include <color.h>

#include "sequenceelement.h"

namespace nap
{
	namespace timeline
	{
		/**
		* SequenceTransition
		*/
		class NAPAPI SequenceTransition : public SequenceElement
		{
			RTTI_ENABLE(SequenceElement)
		public:
			/**
			* Initialize this object after de-serialization
			* @param errorState contains the error message when initialization fails
			*/
			virtual bool init(utility::ErrorState& errorState) override;

			/**
			* Sets the parameter according to the values they are assigned to in this sequence time slot
			* @param time the elapsed time
			* @param endValues a reference to the parameters that need to be set
			* @return returns true if this element has to do something ( element falls in this sequence time slot )
			*/
			virtual bool process(double time, std::vector<Parameter*>& outParameters) override;
		public:
			// properties

			/**
			 * Curve can be nullptr, in which case the transition will use linear interpolation
			 */
			ResourcePtr<math::FloatFCurve> mCurve = nullptr;
		protected:
			/**
			* A vector containing function pointers to the different functions needed to interpolate 
			* between the different parameters
			*/
			std::vector<void(SequenceTransition::*)(
				float progress, 
				const Parameter * inA, 
				const Parameter * inB, 
				Parameter * out)> mFunctions;

			/**
			* The pointer to the evaluation function
			*/
			const float (SequenceTransition::*mEvaluateFunction)(float t);

			/**
			* The internal process function template
			* @param progress the progression, value between 0-1
			* @param inA a pointer to the start parameter
			* @param inB a pointer to the end parameter
			* @param out a pointer to the parameter that will be changed
			*/
			template<typename T1, typename T2>
			void process(float progress, const Parameter* inA, const Parameter* inB, Parameter * out);

			/**
			* Linear interpolation function
			*/
			const float evaluateLinear(float progress) { return progress; }

			/**
			* Use the curve to interpolate
			*/
			const float evaluateCurve(float progress);
		};
	}

	namespace math
	{
		template<>
		NAPAPI nap::RGBColorFloat lerp<nap::RGBColorFloat>(const nap::RGBColorFloat& start, const nap::RGBColorFloat& end, float percent);

		template<>
		NAPAPI nap::RGBAColorFloat lerp<RGBAColorFloat>(const nap::RGBAColorFloat& start, const nap::RGBAColorFloat& end, float percent);

		template<>
		NAPAPI nap::RGBColor8 lerp<RGBColor8>(const nap::RGBColor8& start, const nap::RGBColor8& end, float percent);

		template<>
		NAPAPI nap::RGBAColor8 lerp<RGBAColor8>(const nap::RGBAColor8& start, const nap::RGBAColor8& end, float percent);

		template<>
		NAPAPI glm::ivec4 lerp<glm::ivec4>(const glm::ivec4& start, const glm::ivec4& end, float percent);

		template<>
		NAPAPI glm::ivec3 lerp<glm::ivec3>(const glm::ivec3& start, const glm::ivec3& end, float percent);

		template<>
		NAPAPI glm::ivec2 lerp<glm::ivec2>(const glm::ivec2& start, const glm::ivec2& end, float percent);

		template<>
		NAPAPI int lerp<int>(const int& start, const int& end, float percent);
	}
}
