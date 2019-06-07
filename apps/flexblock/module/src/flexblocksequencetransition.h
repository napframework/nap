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

#include "flexblocksequenceelement.h"

namespace nap
{
	/**
	* FlexBlockSequenceElement
	*/
	class NAPAPI FlexBlockSequenceTransition : public FlexBlockSequenceElement
	{
		RTTI_ENABLE(FlexBlockSequenceElement)
	public:
		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		virtual bool process(double time, std::vector<Parameter*>& outInputs) override;
	public:
		ResourcePtr<math::FloatFCurve> mCurve = nullptr;


	protected:
		std::vector<void(FlexBlockSequenceTransition::*)(float progress, Parameter * inA, Parameter * inB, Parameter * out)> mFunctions;
		float (FlexBlockSequenceTransition::*mEvaluateFunction)(float t);

		template<typename T1, class T2 >
		void process(float progress, Parameter* inA, Parameter* inB, Parameter * out);

		float evaluateLinear(float progress);
		float evaluateCurve(float progress);
	};

	namespace math
	{
		template<>
		NAPAPI nap::RGBColorFloat lerp<nap::RGBColorFloat>(const nap::RGBColorFloat& start, const nap::RGBColorFloat& end, float percent);
	}
}
