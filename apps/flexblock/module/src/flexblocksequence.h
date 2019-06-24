#pragma once

// External Includes
#include <nap/resource.h>

#include "sequence.h"

namespace nap
{
	namespace flexblock
	{
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
		protected:
			/**
			* A vector holder unique pointers to created parameters
			*/
			std::vector <std::unique_ptr<ParameterFloat>> mOwnedParameters;
		};
	}
}
