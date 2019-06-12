#include "sequence.h"

// nap::flexblockstancesequence run time class definition 
RTTI_BEGIN_CLASS(nap::timeline::Sequence)
	// Put additional properties here
	RTTI_PROPERTY("Elements", &nap::timeline::Sequence::mElements, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("StartParameters", &nap::timeline::Sequence::mStartParameters, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace timeline
	{
		Sequence::~Sequence() { }


		bool Sequence::init(utility::ErrorState& errorState)
		{
			if (!errorState.check(mElements.size() > 0,
				"need at least 1 element %s", this->mID.c_str()))
				return false;

			std::vector<ResourcePtr<Parameter>> startParameters = mStartParameters;
			double time = 0.0;
			for (int i = 0; i < mElements.size(); i++)
			{
				mElements[i]->setStartTime(time);
				mElements[i]->setStartParameters(startParameters);

				if (!errorState.check(
					mElements[i]->getStartParameters().size() ==
					startParameters.size(),
					"Start parameters are different %s ", mID.c_str()))
					return false;

				startParameters = mElements[i]->getEndParameters();
				time += mElements[i]->mDuration;

				if (!errorState.check(
					mElements[i]->getEndParameters().size() ==
					startParameters.size(),
					"End parameters are different %s ", mID.c_str()))
					return false;

				for (int j = 0; j < startParameters.size(); j++)
				{
					if (!errorState.check(startParameters[j]->get_type() ==
						mElements[i]->getEndParameters()[j]->get_type(),
						"Parameter types are different type %s and %s do not match in sequence %s ",
						startParameters[j]->mID.c_str(),
						mElements[i]->getEndParameters()[j]->mID.c_str(),
						mID.c_str()))
						return false;
				}
			}

			return true;
		}
	}
}