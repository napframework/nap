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

#include "SequenceTransition.h"

namespace nap
{
	namespace flexblock
	{
		class NAPAPI FlexblockSequenceTransition : public timeline::SequenceTransition
		{
			RTTI_ENABLE(timeline::SequenceTransition)
		public:
			virtual bool init(utility::ErrorState& errorState) override;
		public:
			std::vector<float> mInputs = std::vector<float>(8);
		protected:
			std::vector <std::unique_ptr<ParameterFloat>> mOwnedParameters;
		};
	}
}
