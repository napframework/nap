#pragma once

#pragma once

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <parameternumeric.h>

#include "sequenceelement.h"

namespace nap
{
	namespace timeline
	{
		/**
		* TimelineSequencePause
		*/
		class NAPAPI SequencePause : public SequenceElement
		{
			RTTI_ENABLE(SequenceElement)
		public:
			/**
			* Initialize this object after de-serialization
			* @param errorState contains the error message when initialization fails
			*/
			virtual bool init(utility::ErrorState& errorState) override;

			virtual bool process(double time, std::vector<ResourcePtr<Parameter>> &outParameters) override;
		public:
		};
	}
}
