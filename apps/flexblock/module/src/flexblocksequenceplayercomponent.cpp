#include "flexblocksequenceplayercomponent.h"

// External Includes
#include <entity.h>

// nap::flexblocksequenceplayer run time class definition 
RTTI_BEGIN_CLASS(nap::FlexBlockSequencePlayerComponent)
// Put additional properties here
RTTI_PROPERTY("FlexBlockComponent", &nap::FlexBlockSequencePlayerComponent::mFlexBlockComponent, nap::rtti::EPropertyMetaData::Required)
RTTI_PROPERTY("FlexBlockSequence", &nap::FlexBlockSequencePlayerComponent::mSequence, nap::rtti::EPropertyMetaData::Required)
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

		mSequence = resource->mSequence.get();

		play();

		return true;
	}


	void FlexBlockSequencePlayerComponentInstance::update(double deltaTime)
	{
		if (mIsPlaying)
		{
			if (mCurrentSequenceIndex < mSequence->mElements.size())
			{
				mTime += deltaTime;

				auto currentSequence = mSequence->mElements[mCurrentSequenceIndex];

				if (mTime < currentSequence->mDuration)
				{
					if (currentSequence->getSequenceType() == FlexBlockSequenceElementType::Stance)
					{
						auto stance = currentSequence->getStance();

						for (int i = 0; i < stance->mInputs.size(); i++)
						{
							mFlexBlockComponentInstance->SetMotorInput(i, stance->mInputs[i]);
						}
					}
					else if (currentSequence->getSequenceType() == FlexBlockSequenceElementType::Transition)
					{
						auto stance = currentSequence->getStance();
						auto nextStance = currentSequence->getNextStance();

						for (int i = 0; i < stance->mInputs.size(); i++)
						{
							float a = lerp(stance->mInputs[i], nextStance->mInputs[i], (float)mTime / currentSequence->mDuration);
							mFlexBlockComponentInstance->SetMotorInput(i, a);
						}
					}
				}
				else
				{
					mTime -= currentSequence->mDuration;

					mCurrentSequenceIndex++;

					if (currentSequence->getSequenceType() == FlexBlockSequenceElementType::Stance)
					{
						auto stance = currentSequence->getStance();

						for (int i = 0; i < stance->mInputs.size(); i++)
						{
							mFlexBlockComponentInstance->SetMotorInput(i, stance->mInputs[i]);
						}
					}
					else if (currentSequence->getSequenceType() == FlexBlockSequenceElementType::Transition)
					{
						auto stance = currentSequence->getStance();
						auto nextStance = currentSequence->getNextStance();

						for (int i = 0; i < stance->mInputs.size(); i++)
						{
							float a = lerp(stance->mInputs[i], nextStance->mInputs[i], (float)mTime / currentSequence->mDuration);
							mFlexBlockComponentInstance->SetMotorInput(i, a);
						}
					}
				}
			}
		}
	}

	void FlexBlockSequencePlayerComponentInstance::play()
	{
		mCurrentSequenceIndex = 0;
		mIsPlaying = true;
		mTime = 0.0;
	}

	float FlexBlockSequencePlayerComponentInstance::lerp(float a, float b, float t)
	{
		return a + (b - a) * t;
	}
}