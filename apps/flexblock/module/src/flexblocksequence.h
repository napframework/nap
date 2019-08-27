#pragma once

// External Includes
#include <nap/resource.h>

#include "sequence.h"

namespace nap
{
	namespace flexblock
	{
		enum PARAMETER_IDS
		{
			MOTOR_ONE = 0,
			MOTOR_TWO = 1,
			MOTOR_THREE = 2,
			MOTOR_FOUR = 3,
			MOTOR_FIVE = 4,
			MOTOR_SIX = 5,
			MOTOR_SEVEN = 6,
			MOTOR_EIGHT = 7,
			SLACK = 8,
			MOTOR_OVERRIDE_ONE = 9,
			MOTOR_OVERRIDE_TWO = 10,
			MOTOR_OVERRIDE_THREE = 11,
			MOTOR_OVERRIDE_FOUR = 12,
			MOTOR_OVERRIDE_FIVE = 13,
			MOTOR_OVERRIDE_SIX = 14,
			MOTOR_OVERRIDE_SEVEN = 15,
			MOTOR_OVERRIDE_EIGHT = 16,
			SINUS_FREQUENCY = 17,
			SINUS_AMPLITUDE = 18
		};

		//////////////////////////////////////////////////////////////////////////
		/**
		 * FlexblockSequence
		 * Extends on timeline::Sequence. The mInputs vector will override any assigned startparameters of Sequence
		 *
		 * This class is designed to speed up development of sequences for flexblock project
		 */
		class NAPAPI FlexblockSequence : public timeline::Sequence
		{
			RTTI_ENABLE(timeline::Sequence)
		public:
			virtual ~FlexblockSequence();

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

			std::vector<float> mSpecials = std::vector<float>(11);
		protected:
			/**
			* A vector holder unique pointers to created parameters
			*/
			std::vector <std::unique_ptr<ParameterFloat>> mOwnedParameters;
		};
	}
}
