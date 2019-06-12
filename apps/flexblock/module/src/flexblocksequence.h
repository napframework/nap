#pragma once

// External Includes
#include <nap/resource.h>

#include "sequence.h"

namespace nap
{
	namespace flexblock
	{
		/**
		* TimelineSequence
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
			std::vector<float> mInputs = std::vector<float>(8);
		protected:
			std::vector <std::unique_ptr<ParameterFloat>> mOwnedParameters;
		};
	}
}
