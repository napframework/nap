#include "sequencecontainer.h"

// nap::compositioncontainer run time class definition 
RTTI_BEGIN_CLASS(nap::timeline::SequenceContainer)
RTTI_PROPERTY("Sequences", &nap::timeline::SequenceContainer::mSequences, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace timeline
	{
		SequenceContainer::~SequenceContainer() { }

		bool SequenceContainer::init(utility::ErrorState& errorState)
		{
			if (mSequences.size() > 0)
			{
				const std::vector<Parameter*>& parameters = mSequences[0]->mStartParameters;

				for (int i = 0; i < mSequences.size(); ++i)
				{
					if (!errorState.check(parameters.size() == mSequences[i]->mStartParameters.size(),
						"Size of start parameters of sequences in container differ %s", mID.c_str()))
						return false;

					for (int j = 0; j < parameters.size(); ++j)
					{
						if (!errorState.check( parameters[j]->get_type() == mSequences[i]->mStartParameters[j]->get_type(),
							"Start parameter type of sequences in container differ %s", mID.c_str()))
							return false;
					}
				}
			}

			double time = 0.0;
			for (const auto& sequence : mSequences)
			{
				sequence->setStartTime(time);
				time += sequence->getDuration();
			}

			return true;
		}
	}

}