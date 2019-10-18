// local includes
#include "sequence.h"

// nap::flexblockstancesequence run time class definition 
RTTI_BEGIN_CLASS(nap::timeline::Sequence)
	// Put additional properties here
	RTTI_PROPERTY("Sequence Elements", &nap::timeline::Sequence::mSequenceElementsResourcePtrs, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Start Parameters", &nap::timeline::Sequence::mStartParametersResourcePtrs, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Index", &nap::timeline::Sequence::mIndexInSequenceContainer, nap::rtti::EPropertyMetaData::ReadOnly)
	RTTI_PROPERTY("Name", &nap::timeline::Sequence::mName, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace timeline
	{
		Sequence::~Sequence() { }


		bool Sequence::init(utility::ErrorState& errorState)
		{
			if (!errorState.check(
				mSequenceElementsResourcePtrs.size() > 0 ||
				mElements.size() > 0,
				"Need at least 1 element in sequence %s", this->mID.c_str()))
				return false;

			mElements.clear();
			for (const auto& sequenceElementResourcePtr : mSequenceElementsResourcePtrs)
			{
				mElements.emplace_back(sequenceElementResourcePtr.get());
			}

			sort(mElements.begin(), mElements.end(), [](
				const SequenceElement *a,
				const SequenceElement *b)-> bool
			{
				return a->getStartTime() < b->getStartTime();
			});

			for (const auto& startParameterResourcePtr : mStartParametersResourcePtrs)
			{
				mStartParameters.emplace_back(startParameterResourcePtr.get());
			}

			std::vector<Parameter*> startParameters = mStartParameters;
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


		void Sequence::setStartTime(double startTime)
		{
			mStartTime = startTime;

			std::vector<Parameter*> startParameters = mUseReference ? mStartParametersReference : mStartParameters;
			
			double time = mStartTime;
			for (int i = 0; i < mElements.size(); i++)
			{
				mElements[i]->setStartTime(time);
				mElements[i]->setStartParameters(startParameters);

				startParameters = mElements[i]->getEndParameters();
				time += mElements[i]->mDuration;

				if (i > 0)
				{
					mElements[i]->setPreviousElement(mElements[i - 1]);
				}
			}
			mDuration = time - mStartTime;
		}


		int Sequence::process(double time, std::vector<Parameter*>& outParameters)
		{
			if(time < mStartTime )
				return -1;
			
			if (time >= mStartTime + mDuration)
				return 1;

			for(int i = 0 ; i < mElements.size(); i++)
			{
				if (mElements[mCurrentElementIndex]->process(time, outParameters))
				{
					break;
				}
				else
				{
					mCurrentElementIndex++;
					mCurrentElementIndex %= mElements.size();
				}
			}

			return 0;
		}


		SequenceElement* Sequence::getElementAtTime(const double time) const
		{
			for (int i = 0; i < mElements.size(); i++)
			{
				if (time >= mElements[i]->getStartTime() &&
					time < mElements[i]->getStartTime() + mElements[i]->mDuration)
				{
					return mElements[i];
				}
			}

			return nullptr;
		}

		void Sequence::eraseElements(const int start, const int end)
		{
			assert(start < end);
			assert(end > 0 && end <= mElements.size());
			assert(start < mElements.size() && start > 0);

			mElements.erase(mElements.begin() + start, mElements.begin() + end);
			mCurrentElementIndex = 0;

			std::vector<SequenceElement*> elementsToRemove;
			for (int i = 0; i < mOwnedElements.size(); i++)
			{
				bool owned = false;
				for (int j = 0; j < mElements.size(); j++)
				{
					if (mOwnedElements[i].get() == mElements[j])
					{
						owned = true;
						break;
					}
				}

				if (!owned)
				{
					elementsToRemove.emplace_back(mOwnedElements[i].get());
				}
			}

			auto removeElementLambda = [this](const SequenceElement* sequence)
			{
				for (int i = 0; i < mSequenceElementsResourcePtrs.size(); i++)
				{
					if (mSequenceElementsResourcePtrs[i].get() == sequence)
					{
						mSequenceElementsResourcePtrs.erase(mSequenceElementsResourcePtrs.begin() + i, mSequenceElementsResourcePtrs.begin() + i + 1);
						break;
					}
				}

				for (int i = 0; i < mOwnedElements.size(); i++)
				{
					if (mOwnedElements[i].get() == sequence)
					{
						mOwnedElements.erase(mOwnedElements.begin() + i, mOwnedElements.begin() + i + 1);
						break;
					}
				}
			};

			for (const auto* element : elementsToRemove)
			{
				removeElementLambda(element);
			}
		}

		void Sequence::removeElement(const SequenceElement* element)
		{
			int indexToRemove = -1;
			for (int i = 0; i < mElements.size(); i++)
			{
				if (mElements[i] == element)
				{
					indexToRemove = i;
					break;
				}
			}

			if (indexToRemove >= 0)
			{
				mElements.erase(mElements.begin() + indexToRemove);
			}

			indexToRemove = -1;
			for (int i = 0; i < mSequenceElementsResourcePtrs.size(); i++)
			{
				if (mSequenceElementsResourcePtrs[i].get() == element)
				{
					indexToRemove = i;
					break;
				}
			}

			if (indexToRemove >= 0)
			{
				mSequenceElementsResourcePtrs.erase(mSequenceElementsResourcePtrs.begin() + indexToRemove);
			}

			indexToRemove = -1;
			for (int i = 0; i < mOwnedElements.size(); i++)
			{
				if (mOwnedElements[i].get() == element)
				{
					indexToRemove = i;
					break;;
				}
			}

			if (indexToRemove >= 0)
			{
				mOwnedElements.erase(mOwnedElements.begin() + indexToRemove);
			}

			mCurrentElementIndex = 0;
		}

		SequenceElement* Sequence::insertElement(std::unique_ptr<SequenceElement> element)
		{
			ResourcePtr<SequenceElement> elementResourcePtr(element.get());
			mSequenceElementsResourcePtrs.emplace_back(elementResourcePtr);

			mElements.emplace_back(element.get());

			sort(mElements.begin(), mElements.end(), [](
				const SequenceElement *a,
				const SequenceElement *b)-> bool
			{
				return a->getStartTime() < b->getStartTime();
			});

			for (int i = 1; i < mElements.size(); i++)
			{
				mElements[i]->setPreviousElement(mElements[i - 1]);
			}

			mCurrentElementIndex = 0;

			mOwnedElements.emplace_back(std::move(element));

			return mOwnedElements.back().get();
		}
	}
}