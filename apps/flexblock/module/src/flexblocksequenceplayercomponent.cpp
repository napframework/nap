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
			mTime += deltaTime;

			while (!mSequence->mElements[mCurrentSequenceIndex]->process(mTime, mInputs) )
			{
				mCurrentSequenceIndex++;

				if (mCurrentSequenceIndex >= mSequence->mElements.size())
				{
					mIsPlaying = false;
					break;
				}
			}

			for (int i = 0; i < mInputs.size(); i++)
			{
				mFlexBlockComponentInstance->SetMotorInput(i, mInputs[i]);
			}
		}
	}

	void FlexBlockSequencePlayerComponentInstance::play()
	{
		mCurrentSequenceIndex = 0;
		mIsPlaying = true;
		mTime = 0.0;
	}
}