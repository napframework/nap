#include "SequencePause.h"

// nap::FlexBlockSequenceElement run time class definition 
RTTI_BEGIN_CLASS(nap::timeline::SequencePause)
// Put additional properties here
RTTI_PROPERTY("Duration", &nap::timeline::SequenceElement::mDuration, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace timeline
	{
		bool SequencePause::init(utility::ErrorState& errorState)
		{
			return SequenceElement::init(errorState);
		}

		bool SequencePause::process(double time, std::vector<Parameter*>& outParameters)
		{
			if (!SequenceElement::process(time, outParameters))
				return false;

			for (int i = 0; i < mStartParameters.size(); i++)
			{
				outParameters[i]->setValue(*mStartParameters[i]);
			}

			return true;
		}

		void SequencePause::setStartParameters(const std::vector<Parameter*>& startParameters)
		{
			mStartParameters = startParameters;
			mEndParameters = startParameters;
		}
	}
}