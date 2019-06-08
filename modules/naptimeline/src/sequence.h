#pragma once

// External Includes
#include <nap/resource.h>

#include "sequenceelement.h"

namespace nap
{
	namespace timeline
	{
		/**
		* TimelineSequence
		*/
		class NAPAPI Sequence : public Resource
		{
			RTTI_ENABLE(Resource)
		public:
			virtual ~Sequence();

			/**
			* Initialize this object after de-serialization
			* @param errorState contains the error message when initialization fails
			*/
			virtual bool init(utility::ErrorState& errorState) override;
		public:
			// properties
			std::vector<ResourcePtr<SequenceElement>> mElements;

			//
			std::vector<Parameter*> mStartParameters;
		};
	}
}
