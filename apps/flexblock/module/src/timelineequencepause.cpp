#include "timelinesequencepause.h"

// nap::FlexBlockSequenceElement run time class definition 
RTTI_BEGIN_CLASS(nap::FlexBlockSequencePause)
// Put additional properties here
RTTI_PROPERTY("Duration", &nap::TimelineSequenceElement::mDuration, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{

	bool FlexBlockSequencePause::init(utility::ErrorState& errorState)
	{
		if (!TimelineSequenceElement::init(errorState))
			return false;

		return true;
	}

	bool FlexBlockSequencePause::process(double time, std::vector<Parameter*>& outParameters)
	{
		if (!TimelineSequenceElement::process(time, outParameters))
			return false;

		for (int i = 0; i < mStartParameters.size(); i++)
		{
			outParameters[i]->setValue(*mStartParameters[i]);
		}

		return true;
	}

	void FlexBlockSequencePause::setStartParameters(const std::vector<Parameter*>& startParameters) 
	{
		mParameters = startParameters;
	}
}