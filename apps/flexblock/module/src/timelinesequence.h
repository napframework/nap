#pragma once

// External Includes
#include <nap/resource.h>

#include "timelinesequenceelement.h"

namespace nap
{
	/**
	 * TimelineSequence
	 */
	class NAPAPI TimelineSequence : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~TimelineSequence();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;
	public:
		// properties
		std::vector<ResourcePtr<TimelineSequenceElement>> mElements;

		//
		std::vector<Parameter*> mStartParameters;
	};
}
