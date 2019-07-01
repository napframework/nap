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
		//////////////////////////////////////////////////////////////////////////

		/**
		 * SequenceTransition
		 * Describes a transition from given start parameters to given end parameters
		 * Can have a fcurve assigned which it will use to evaluate the transition over given duration
		 * I no fcurve is assigned, a linear interpolation between start and end will take place
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

			const std::vector<ResourcePtr<math::FloatFCurve>>& getCurves() { return mCurves; }
		public:
			// properties

			/**
			 * Array of curves, each parameter can have its own curve
			 */
			std::vector<ResourcePtr<math::FloatFCurve>> mCurves; ///< Property: 'Curves' array of curves this transition holds per parameter
		protected:
			/**
			* A vector containing function pointers to the different functions needed to interpolate 
			* between the different parameters
			*/
			std::vector<void(SequenceTransition::*)(
				float progress, 
				const Parameter & inA, 
				const Parameter & inB, 
				Parameter & out,
				const int curveIndex)> mFunctions;

			/**
			* The pointer to the evaluation function
			*/
			const float (SequenceTransition::*mEvaluateFunction)(float t, const int curveIndex);

			/**
			* The internal process function template
			* @param progress the progression, value between 0-1
			* @param inA a reference to the start parameter
			* @param inB a reference to the end parameter
			* @param out a reference to the parameter that will be changed
			* @param the curve index this parameter is assigned to
			*/
			template<typename T1, typename T2>
			void process(float progress, const Parameter& inA, const Parameter& inB, Parameter & out, const int curveIndex);

			/**
			* Linear interpolation function
			*/
			const float evaluateLinear(float progress) { return progress; }

			/**
			* Use the curve to interpolate
			*/
			const float evaluateCurve(float progress, const int curveIndex);

			std::vector<std::unique_ptr<math::FloatFCurve>> mOwnedCurves;
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
