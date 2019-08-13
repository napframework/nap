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

#include "sequencetransition.h"

namespace nap
{
	namespace flexblock
	{
		//////////////////////////////////////////////////////////////////////////

		/**
		 * FlexblockSequenceTransition
		 * Extends on timeline::SequenceTransition. The mInputs vector will override any assigned parameters of this
		 * sequence and create new parameter resources during init. Pointers to new parameters
		 * will be owned by the instance of this class.
		 * 
		 * This class is designed to make creating sequences for FlexBlock easier, giving you a list of 8 inputs to start from.
		 */
		class NAPAPI FlexblockSequenceTransition : public timeline::SequenceTransition
		{
			RTTI_ENABLE(timeline::SequenceTransition)
		public:
			/**
			 * Initialize this object after de-serialization
			 * @param errorState contains the error message when initialization fails
			 */
			virtual bool init(utility::ErrorState& errorState) override;
		public:
			/**
			 * 8 floating point values that will be used to create parameter resources during init
			 */
			std::vector<float> mMotorInputs = std::vector<float>(8);

			float mSlack = 0.0f;
		protected:
			/**
			 * A vector holder unique pointers to created parameters
			 */
			std::vector <std::unique_ptr<ParameterFloat>> mOwnedParameters;
		};
	}
}
