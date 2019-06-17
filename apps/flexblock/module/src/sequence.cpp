#include "sequence.h"

// nap::flexblockstancesequence run time class definition 
RTTI_BEGIN_CLASS(nap::timeline::Sequence)
	// Put additional properties here
	RTTI_PROPERTY("Sequence Elements", &nap::timeline::Sequence::mSequenceElements, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Start Parameters", &nap::timeline::Sequence::mStartParameters, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace timeline
	{
		Sequence::~Sequence() { }


		bool Sequence::init(utility::ErrorState& errorState)
		{
			if (!errorState.check(mSequenceElements.size() > 0,
				"need at least 1 element %s", this->mID.c_str()))
				return false;

			std::vector<Parameter*> startParameters = mStartParameters;
			double time = 0.0;
			for (int i = 0; i < mSequenceElements.size(); i++)
			{
				mSequenceElements[i]->setStartTime(time);
				mSequenceElements[i]->setStartParameters(startParameters);

				if (!errorState.check(
					mSequenceElements[i]->getStartParameters().size() ==
					startParameters.size(),
					"Start parameters are different %s ", mID.c_str()))
					return false;

				startParameters = mSequenceElements[i]->getEndParameters();
				time += mSequenceElements[i]->mDuration;

				if (!errorState.check(
					mSequenceElements[i]->getEndParameters().size() ==
					startParameters.size(),
					"End parameters are different %s ", mID.c_str()))
					return false;

				for (int j = 0; j < startParameters.size(); j++)
				{
					if (!errorState.check(startParameters[j]->get_type() ==
						mSequenceElements[i]->getEndParameters()[j]->get_type(),
						"Parameter types are different type %s and %s do not match in sequence %s ",
						startParameters[j]->mID.c_str(),
						mSequenceElements[i]->getEndParameters()[j]->mID.c_str(),
						mID.c_str()))
						return false;
				}
			}

			return true;
		}

		void Sequence::setStartTime(double startTime)
		{
			mStartTime = startTime;

			std::vector<Parameter*> startParameters = mStartParameters;
			
			double time = mStartTime;
			for (int i = 0; i < mSequenceElements.size(); i++)
			{
				mSequenceElements[i]->setStartTime(time);
				mSequenceElements[i]->setStartParameters(startParameters);

				startParameters = mSequenceElements[i]->getEndParameters();
				time += mSequenceElements[i]->mDuration;
			}
			mDuration = time - mStartTime;
		}

		void Sequence::reset()
		{
			mCurrentElementIndex = 0;
		}

		int Sequence::process(double time, std::vector<Parameter*>& outParameters)
		{
			if(time < mStartTime )
				return -1;
			
			if (time >= mStartTime + mDuration)
				return 1;

			for(int i = 0 ; i < mSequenceElements.size(); i++)
			{
				if (mSequenceElements[mCurrentElementIndex]->process(time, outParameters))
				{
					break;
				}
				else
				{
					mCurrentElementIndex++;
					mCurrentElementIndex %= mSequenceElements.size();
				}
			}

			return 0;
		}
	}
}