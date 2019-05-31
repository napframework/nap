#pragma once

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <parameternumeric.h>

#include "flexblockkeyframe.h"

namespace nap
{
	/**
	 * FlexBlockSequenceElement
	 */
	class NAPAPI FlexBlockSequenceElement : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~FlexBlockSequenceElement();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		* Initialize this object after de-serialization
		* @param time the elapsed time
		* @param outInputs a reference to the inputs that need to be set
		* @return returns true if this element has done something
		*/
		virtual bool process(double time, std::vector<ParameterFloat*>& outInputs);

		/**
		 * This is called by the sequence to set the start time of this element
		 */
		void setStartTime(double startTime);
	public:
		// properties
		float mDuration = 0.0f;
		ResourcePtr<FlexBlockKeyFrame> mKeyFrame = nullptr;
	protected:
		double mStartTime = 0.0;
	};
}
