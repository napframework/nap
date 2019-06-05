#pragma once

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <fcurve.h>
#include <math.h>
#include <parameternumeric.h>

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

		virtual bool process(double time, std::vector<ParameterFloat*>& outInputs) override;
	public:
		ResourcePtr<math::FloatFCurve> mCurve = nullptr;
	protected:
		bool (FlexBlockSequenceTransition::*mProcessFunc)(double time, std::vector<ParameterFloat*>& outInputs) = nullptr;

		bool processWithCurve(double time, std::vector<ParameterFloat*>& outInputs);
		bool processLinear(double time, std::vector<ParameterFloat*>& outInputs);
	};
}
