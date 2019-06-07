#include "flexblocksequenceplayercomponent.h"

// External Includes
#include <entity.h>

// nap::flexblocksequenceplayer run time class definition 
RTTI_BEGIN_CLASS(nap::FlexBlockSequencePlayerComponent)
// Put additional properties here
RTTI_PROPERTY("FlexBlockParameters", &nap::FlexBlockSequencePlayerComponent::mParameterGroup, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::flexblocksequenceplayerInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FlexBlockSequencePlayerComponentInstance)
RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	void FlexBlockSequencePlayerComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool FlexBlockSequencePlayerComponentInstance::init(utility::ErrorState& errorState)
	{
		FlexBlockSequencePlayerComponent* resource = getComponent<FlexBlockSequencePlayerComponent>();

		const auto& parameterGroup = resource->mParameterGroup;
		for (const auto& parameter : parameterGroup->mParameters)
		{
			mParameters.emplace_back(parameter.get());
		}

		return true;
	}

	const std::vector<ResourcePtr<FlexBlockSequenceElement>>& FlexBlockSequencePlayerComponentInstance::getElements()
	{
		assert(mSequence != nullptr);
		return mSequence->mElements;
	}

	void FlexBlockSequencePlayerComponentInstance::update(double deltaTime)
	{
		if (mIsPlaying)
		{
			if (!mIsPaused)
			{
				mTime += deltaTime;
			}

			while (!mSequence->mElements[mCurrentSequenceIndex]->process(mTime, mParameters) )
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

	void FlexBlockSequencePlayerComponentInstance::play()
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

	void FlexBlockSequencePlayerComponentInstance::setTime(double time)
	{
		mTime = time;
		mCurrentSequenceIndex = 0;
	}

	void FlexBlockSequencePlayerComponentInstance::pause()
	{
		mIsPaused = true;
	}

	bool FlexBlockSequencePlayerComponentInstance::load(ResourcePtr<FlexBlockSequence> sequence, utility::ErrorState& error)
	{
		if (!error.check(sequence != nullptr,
			"Sequence is null %s", mID.c_str()))
			return false;

		if (!error.check(
			mParameters.size() ==
			sequence->mStartParameters.size(),
			"Parameters are different %s ", mID.c_str()))
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

	void FlexBlockSequencePlayerComponentInstance::stop()
	{
		mIsPlaying = false;
		mIsPaused = false;
		mIsFinished = false;
	}
}