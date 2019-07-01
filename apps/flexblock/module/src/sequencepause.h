#pragma once

#pragma once

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <parameternumeric.h>

#include "sequenceelement.h"
#include "sequence.h"

namespace nap
{
	namespace timeline
	{
		//////////////////////////////////////////////////////////////////////////

		/**
		 * SequencePause
		 * A SequencePause just sets the parameters of to the values of the previous sequence element
		 */
		class NAPAPI SequencePause : public SequenceElement
		{
			friend class Sequence;

			RTTI_ENABLE(SequenceElement)
		public:
			/**
			 * Initialize this object after de-serialization
			 * @param errorState contains the error message when initialization fails
			 */
			virtual bool init(utility::ErrorState& errorState) override;

			/**
			 * Sets the parameter according to the values they are assigned to in this timeslot
			 * @param time the elapsed time
			 * @param endValues a reference to the parameters that need to be set
			 * @return returns true if this element has to do something ( element falls in this timeframe )
			 */
			virtual bool process(double time, std::vector<Parameter*>& outParameters) override;
		public:
			/**
			 * Set the start parameters of this time slot, this is set by the sequence and usually reference the parameters
			 * of the sequence before this one
			 * @param startParameters the start parameters
			 */
			virtual void setStartParameters(const std::vector<Parameter*>& startParameters) override;
		};
	}
}
