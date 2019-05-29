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

		return true;
	}


	void FlexBlockSequencePlayerComponentInstance::update(double deltaTime)
	{
		if (mIsPlaying)
		{
			mTime += deltaTime;

			float pos = (float)mTime / mCurrentSequenceElement->mDuration;
		}
	}

	void FlexBlockSequencePlayerComponentInstance::play()
	{
		mCurrentSequenceElement = mSequence->mElements[0].get();
		mIsPlaying = true;
		mTime = 0.0;
	}
}