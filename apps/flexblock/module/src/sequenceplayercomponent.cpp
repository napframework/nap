#include "sequenceplayercomponent.h"

// External Includes
#include <entity.h>

// nap::flexblocksequenceplayer run time class definition 
RTTI_BEGIN_CLASS(nap::timeline::SequencePlayerComponent)
// Put additional properties here
RTTI_PROPERTY("TimelineParameters", &nap::timeline::SequencePlayerComponent::mParameterGroup, nap::rtti::EPropertyMetaData::Required)
RTTI_PROPERTY("SequenceContainer", &nap::timeline::SequencePlayerComponent::mSequenceContainer, nap::rtti::EPropertyMetaData::Required)
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

		void SequencePlayerComponentInstance::skipToSequence(Sequence * sequence)
		{
			for (const auto* sequence_ : mSequenceContainer->mSequences)
			{
				if (sequence_ == sequence)
				{
					mSequenceContainer->mSequences[mCurrentSequenceIndex]->reset();
					mTime = sequence->getStartTime();
					mCurrentSequenceIndex = 0;
					mSequenceContainer->mSequences[mCurrentSequenceIndex]->reset();
					break;
				}
			}
		}

		bool SequencePlayerComponentInstance::init(utility::ErrorState& errorState)
		{
			SequencePlayerComponent* resource = getComponent<SequencePlayerComponent>();

			const auto& parameterGroup = resource->mParameterGroup;
			for (const auto& parameter : parameterGroup->mParameters)
			{
				mParameters.emplace_back(parameter.get());
			}

			mSequenceContainer = resource->mSequenceContainer.get();

			double time = 0.0;
			for (const auto& sequence : mSequenceContainer->mSequences)
			{
				sequence->setStartTime(time);
				time += sequence->getDuration();
			}
			mDuration = time;

			return true;
		}

		const std::vector<Sequence*>& SequencePlayerComponentInstance::getSequences()
		{
			return mSequenceContainer->mSequences;
		}

		void SequencePlayerComponentInstance::update(double deltaTime)
		{
			if (mIsPlaying)
			{
				if (!mIsPaused)
				{
					mTime += deltaTime;
				}

				while (!mSequenceContainer->mSequences[mCurrentSequenceIndex]->process(mTime, mParameters))
				{
					mSequenceContainer->mSequences[mCurrentSequenceIndex]->reset();
					mCurrentSequenceIndex++;

					if (mCurrentSequenceIndex >= mSequenceContainer->mSequences.size())
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
						mSequenceContainer->mSequences[mCurrentSequenceIndex]->reset();

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

		void SequencePlayerComponentInstance::stop()
		{
			mIsPlaying = false;
			mIsPaused = false;
			mIsFinished = false;
			mCurrentSequenceIndex = 0;
			mTime = 0.0;
		}
	}
}