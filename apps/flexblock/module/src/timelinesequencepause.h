#pragma once

#pragma once

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <parameternumeric.h>

#include "timelinesequenceelement.h"

namespace nap
{
	/**
	* FlexBlockSequenceElement
	*/
	class NAPAPI FlexBlockSequencePause : public TimelineSequenceElement
	{
		RTTI_ENABLE(TimelineSequenceElement)
	public:
		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		virtual bool process(double time, std::vector<Parameter*> &outParameters) override;

		virtual void setStartParameters(const std::vector<Parameter*>& startParameters) override;
	public:
	};
}
