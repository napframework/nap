#include "sequencecontainer.h"

// nap::compositioncontainer run time class definition 
RTTI_BEGIN_CLASS(nap::timeline::SequenceContainer)
RTTI_PROPERTY("Sequences", &nap::timeline::SequenceContainer::mSequenceLinks, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace timeline
	{
		SequenceContainer::~SequenceContainer() { }

		bool SequenceContainer::init(utility::ErrorState& errorState)
		{
			if (mSequenceLinks.size() > 0)
			{
				const std::vector<Parameter*>& parameters = mSequenceLinks[0]->mStartParameters;

				for (int i = 0; i < mSequenceLinks.size(); ++i)
				{
					if (!errorState.check(parameters.size() == mSequenceLinks[i]->mStartParameters.size(),
						"Size of start parameters of sequences in container differ %s", mID.c_str()))
						return false;

					for (int j = 0; j < parameters.size(); ++j)
					{
						if (!errorState.check( parameters[j]->get_type() == mSequenceLinks[i]->mStartParameters[j]->get_type(),
							"Start parameter type of sequences in container differ %s", mID.c_str()))
							return false;
					}
				}
			}

			double time = 0.0;
			mSequences.clear();
			for (const auto& sequence : mSequenceLinks)
			{
				sequence->setStartTime(time);
				time += sequence->getDuration();
				mSequences.emplace_back(sequence.get());
			}

			for (int i = 0; i < mSequences.size(); i++)
			{
				mSequences[i]->mIndexInSequenceContainer = i;
			}

			for (int i = 1; i < mSequences.size(); i++)
			{
				mSequences[i]->mElements[0]->setPreviousElement(mSequences[i - 1]->mElements.back());
			}

			return true;
		}

		void SequenceContainer::reinit()
		{
			double time = 0.0;
			for (int i = 0; i < mSequences.size(); i++)
			{
				if (i > 0)
				{
					mSequences[i]->mStartParameters.clear();
					mSequences[i]->mStartParametersReference = mSequences[i - 1]->mElements.back()->getEndParameters();
					mSequences[i]->mUseReference = true;
				}
				else
				{
					mSequences[i]->mUseReference = false;
				}

				mSequences[i]->setStartTime(time);
				time += mSequences[i]->getDuration();
			}

			for (int i = 1; i < mSequences.size(); i++)
			{
				mSequences[i]->mElements[0]->setPreviousElement(mSequences[i - 1]->mElements.back());
			}
		}

		void SequenceContainer::reconstruct()
		{
			double time = 0.0;
			for (int i = 0; i < mSequences.size(); i++)
			{
				if (i > 0)
				{
					mSequences[i]->mStartParameters.clear();
					mSequences[i]->mStartParametersReference = mSequences[i - 1]->mElements.back()->getEndParameters();
					mSequences[i]->mUseReference = true;
				}
				else
				{
					mSequences[i]->mUseReference = false;
				}

				mSequences[i]->setStartTime(time);
				time += mSequences[i]->getDuration();
			}
		}
	}

}