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

		auto parameterGroup = resource->mParameterGroup;
		if (!errorState.check(parameterGroup->mParameters.size() == 8,
			"parameter group must have 8 parameters %s", this->mID.c_str()))
			return false;
		
		for (const auto& parameter : parameterGroup->mParameters)
		{
			ParameterFloat *parameterFloat = dynamic_cast<ParameterFloat*>(parameter.get());
			
			if (!errorState.check(parameterFloat != nullptr,
				"parameter must be of type parameterfloat %s", parameter->mID.c_str()))
				return false;
			
			mInputs.emplace_back(parameterFloat);
		}

		return true;
	}

	const std::vector<FlexBlockSequenceElement*> FlexBlockSequencePlayerComponentInstance::getElements()
	{
		std::vector<FlexBlockSequenceElement*> elements;
		
		if (mSequence != nullptr)
		{
			for (const auto& element : mSequence->mElements)
			{
				elements.emplace_back(element.get());
			}
		}

		return elements;
	}

	void FlexBlockSequencePlayerComponentInstance::update(double deltaTime)
	{
		if (mIsPlaying)
		{
			if (!mIsPaused)
			{
				mTime += deltaTime;
			}

			while (!mSequence->mElements[mCurrentSequenceIndex]->process(mTime, mInputs) )
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

	void FlexBlockSequencePlayerComponentInstance::load(FlexBlockSequence* sequence)
	{
		if (mSequence != sequence)
		{
			stop();

			mSequence = sequence;

			mTime = 0.0;
			mDuration = 0.0;

			for (auto& element : mSequence->mElements)
			{
				mDuration += element->mDuration;
			}
		}
	}

	void FlexBlockSequencePlayerComponentInstance::stop()
	{
		mIsPlaying = false;
		mIsPaused = false;
		mIsFinished = false;
	}
}