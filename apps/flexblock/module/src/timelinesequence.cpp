#include "timelinesequence.h"

// nap::flexblockstancesequence run time class definition 
RTTI_BEGIN_CLASS(nap::TimelineSequence)
	// Put additional properties here
	RTTI_PROPERTY("Elements", &nap::TimelineSequence::mElements, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("StartInputs", &nap::TimelineSequence::mStartParameters, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	TimelineSequence::~TimelineSequence()			{ }


	bool TimelineSequence::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mElements.size() > 0,
			"need at least 1 element %s", this->mID.c_str()))
			return false;

		std::vector<Parameter*>& startParameters = mStartParameters;
		double time = 0.0;
		for (int i = 0; i < mElements.size(); i++)
		{
			mElements[i]->setStartTime(time);
			mElements[i]->setStartParameters(startParameters);
			startParameters = mElements[i]->getParameters();
			time += mElements[i]->mDuration;

			if (!errorState.check(
				mElements[i]->getParameters().size() ==
				startParameters.size(),
				"Parameters are different %s ", mID.c_str()))
				return false;

			for (int j = 0; j < startParameters.size(); j++)
			{
				if (!errorState.check(startParameters[j]->get_type() ==
					mElements[i]->getParameters()[j]->get_type(),
					"Parameter types are different type %s and %s do not match in sequence %s ",
					startParameters[j]->mID.c_str(),
					mElements[i]->getParameters()[j]->mID.c_str(),
					mID.c_str()))
					return false;
			}
		}

		return true;
	}
}