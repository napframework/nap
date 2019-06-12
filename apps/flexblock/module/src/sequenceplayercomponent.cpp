#include "sequenceplayercomponent.h"

// External Includes
#include <entity.h>

// nap::flexblocksequenceplayer run time class definition 
RTTI_BEGIN_CLASS(nap::timeline::SequencePlayerComponent)
// Put additional properties here
RTTI_PROPERTY("TimelineParameters", &nap::timeline::SequencePlayerComponent::mParameterGroup, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::flexblocksequenceplayerInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::timeline::SequencePlayerComponentInstance)
RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	namespace timeline
	{
		void SequencePlayerComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
		{

		}


		bool SequencePlayerComponentInstance::init(utility::ErrorState& errorState)
		{
			SequencePlayerComponent* resource = getComponent<SequencePlayerComponent>();

			const auto& parameterGroup = resource->mParameterGroup;
			for (const auto& parameter : parameterGroup->mParameters)
			{
				mParameters.emplace_back(parameter.get());
			}

			return true;
		}

		const std::vector<ResourcePtr<SequenceElement>>& SequencePlayerComponentInstance::getElements()
		{
			assert(mSequence != nullptr);
			return mSequence->mElements;
		}

		void SequencePlayerComponentInstance::update(double deltaTime)
		{
			if (mIsPlaying)
			{
				if (!mIsPaused)
				{
					mTime += deltaTime;
				}

				while (!mSequence->mElements[mCurrentSequenceIndex]->process(mTime, mParameters))
				{
					mCurrentSequenceIndex++;

					if (mCurrentSequenceIndex >= mSequence->mElements.size())
					{
						if (!mIsPaused)
						{
							if (mIsLooping)
							{
								mTime = 0.0;
								mCurrentSequenceIndex = 0;
								break;
							}
							else
							{
								mIsPlaying = false;
								mIsFinished = true;
							}
						}

						mCurrentSequenceIndex -= 1;

						break;
					}
				}
			}
		}

		void SequencePlayerComponentInstance::play()
		{
			if (mIsPlaying)
			{
				mIsPaused = false;
			}
			else
			{
				mCurrentSequenceIndex = 0;
				mIsPlaying = true;
				mIsFinished = false;
				mTime = 0.0;
			}
		}

		void SequencePlayerComponentInstance::setTime(double time)
		{
			mTime = math::clamp<double>(time, 0.0, mDuration);
			mCurrentSequenceIndex = 0;
		}

		void SequencePlayerComponentInstance::pause()
		{
			mIsPaused = true;
		}

		bool SequencePlayerComponentInstance::load(ResourcePtr<Sequence> sequence, utility::ErrorState& error)
		{
			if (!error.check(sequence != nullptr,
				"Sequence is null %s", mID.c_str()))
				return false;

			if (!error.check(
				mParameters.size() ==
				sequence->mStartParameters.size(),
				"Timeline Parameters sizes are different then those of sequence %s \n", mID.c_str()))
				return false;

			for (int i = 0; i < mParameters.size(); i++)
			{
				for (int j = 0; j < sequence->mStartParameters.size(); j++)
				{
					if (!error.check(mParameters[j]->get_type() ==
						sequence->mStartParameters[j]->get_type(),
						"Parameter types are different type %s and %s do not match in sequence %s ",
						mParameters[j]->mID.c_str(),
						sequence->mStartParameters[j]->mID.c_str(),
						mID.c_str()))
						return false;
				}
			}

			stop();

			mSequence = sequence.get();

			mTime = 0.0;
			mDuration = 0.0;

			for (const auto& element : mSequence->mElements)
			{
				mDuration += element->mDuration;
			}

			return true;
		}

		void SequencePlayerComponentInstance::stop()
		{
			mIsPlaying = false;
			mIsPaused = false;
			mIsFinished = false;
			mSequence = nullptr;
		}
	}
}