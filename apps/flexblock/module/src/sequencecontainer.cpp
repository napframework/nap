// local includes
#include "sequencecontainer.h"

// nap::compositioncontainer run time class definition 
RTTI_BEGIN_CLASS(nap::timeline::SequenceContainer)
RTTI_PROPERTY("Sequences", &nap::timeline::SequenceContainer::mSequenceResourcePtrs, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace timeline
	{
		SequenceContainer::~SequenceContainer() { }

		bool SequenceContainer::init(utility::ErrorState& errorState)
		{
			if (mSequenceResourcePtrs.size() > 0)
			{
				const std::vector<Parameter*>& parameters = mSequenceResourcePtrs[0]->mStartParameters;

				for (int i = 0; i < mSequenceResourcePtrs.size(); ++i)
				{
					if (!errorState.check(parameters.size() == mSequenceResourcePtrs[i]->mStartParameters.size(),
						"Size of start parameters of sequences in container differ %s", mID.c_str()))
						return false;

					for (int j = 0; j < parameters.size(); ++j)
					{
						if (!errorState.check( parameters[j]->get_type() == mSequenceResourcePtrs[i]->mStartParameters[j]->get_type(),
							"Start parameter type of sequences in container differ %s", mID.c_str()))
							return false;
					}
				}
			}

			// sort the list of sequences
			sort(mSequenceResourcePtrs.begin(), mSequenceResourcePtrs.end(), [](
				const nap::ResourcePtr<Sequence> a,
				const nap::ResourcePtr<Sequence> b)-> bool
			{
				return a.get()->mIndexInSequenceContainer < b.get()->mIndexInSequenceContainer;
			});


			double time = 0.0;
			mSequences.clear();
			for (const auto& sequence : mSequenceResourcePtrs)
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
				mSequences[i]->getElements()[0]->setPreviousElement(mSequences[i - 1]->getElements().back());
			}

			return true;
		}

	
		void SequenceContainer::removeSequence(const Sequence * sequence)
		{
			int indexToRemove = -1;
			for (int i = 0; i < mSequences.size(); i++)
			{
				if (sequence == mSequences[i])
				{
					indexToRemove = i;
					break;
				}
			}

			if (indexToRemove > -1)
			{
				mSequences.erase(mSequences.begin() + indexToRemove);
			}

			for (int i = 0; i < mOwnedSequences.size(); i++)
			{
				if (mOwnedSequences[i].get() == sequence)
				{
					mOwnedSequences.erase(mOwnedSequences.begin() + i);
					break;
				}
			}

			reconstruct();
		}


		void SequenceContainer::removeSequenceElement(const Sequence * sequence, const SequenceElement * element)
		{
			for (int i = 0; i < mSequences.size(); i++)
			{
				if (mSequences[i] == sequence)
				{
					mSequences[i]->removeElement(element);
					break;;
				}
			}
		}


		Sequence* SequenceContainer::insertSequence(std::unique_ptr<Sequence> sequence)
		{
			Sequence* returnSequenecePtr = sequence.get();
			mSequences.emplace_back(sequence.get());
			mOwnedSequences.emplace_back(std::move(sequence));

			reconstruct();

			return returnSequenecePtr;
		}


		void SequenceContainer::setSequences(std::vector<std::unique_ptr<Sequence>>& sequences)
		{
			mSequences.clear();
			mOwnedSequences.clear();
			for (std::unique_ptr<Sequence>& sequence : sequences)
			{
				mSequences.emplace_back(sequence.get());
				mOwnedSequences.emplace_back(std::move(sequence));
			}

			reconstruct();
		}


		void SequenceContainer::clearSequences()
		{
			mSequences.clear();
			mOwnedSequences.clear();
		}


		void SequenceContainer::reconstruct()
		{
			// sort the list of sequences
			sort(mSequences.begin(), mSequences.end(), [](
				const Sequence *a,
				const Sequence *b)-> bool
			{
				return a->getStartTime() < b->getStartTime();
			});

			double time = 0.0;
			SequenceElement* lastElement = nullptr;

			for (int i = 0; i < mSequences.size(); i++)
			{
				if (i > 0)
				{
					mSequences[i]->mStartParameters.clear();
					mSequences[i]->mStartParametersReference = mSequences[i - 1]->getElements().back()->getEndParameters();
					mSequences[i]->mUseReference = true;
				}
				else
				{
					mSequences[i]->mUseReference = false;
				}

				
				mSequences[i]->mIndexInSequenceContainer = i;

				mSequences[i]->setStartTime(time);
				time += mSequences[i]->getDuration();

				for (int j = 0; j < mSequences[i]->getElements().size(); j++)
				{
					mSequences[i]->getElements()[j]->setPreviousElement(lastElement);
					lastElement = mSequences[i]->getElements()[j];
				}
			}
		}
	}
}