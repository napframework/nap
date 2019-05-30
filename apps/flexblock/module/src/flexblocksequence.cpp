#include "flexblocksequence.h"

// nap::flexblockstancesequence run time class definition 
RTTI_BEGIN_CLASS(nap::FlexBlockSequence)
	// Put additional properties here
	RTTI_PROPERTY("Elements", &nap::FlexBlockSequence::mElements, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	FlexBlockSequence::~FlexBlockSequence()			{ }


	bool FlexBlockSequence::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mElements.size() > 0,
			"need at least 1 element %s", this->mID.c_str()))
			return false;

		double time = 0.0;
		for (int i = 0; i < mElements.size(); i++)
		{
			mElements[i]->setStartTime(time);
			time += mElements[i]->mDuration;
		}

		return true;
	}
}