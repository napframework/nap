#include "flexblocksequenceplayercomponent.h"

// External Includes
#include <entity.h>

// nap::flexblocksequenceplayer run time class definition 
RTTI_BEGIN_CLASS(nap::FlexBlockSequencePlayerComponent)
// Put additional properties here
RTTI_PROPERTY("FlexBlockParameters", &nap::FlexBlockSequencePlayerComponent::mParameterGroup, nap::rtti::EPropertyMetaData::Required)
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

		auto parameterGroup = resource->mParameterGroup;
		if (!errorState.check(parameterGroup->mParameters.size() == 8,
			"parameter group must have 8 parameters %s", this->mID.c_str()))
			return false;
		
		for (auto parameter : parameterGroup->mParameters)
		{
			ParameterFloat *parameterFloat = dynamic_cast<ParameterFloat*>(parameter.get());
			
			if (!errorState.check(parameterFloat != nullptr,
				"parameter must be of type parameterfloat %s", parameter->mID.c_str()))
				return false;
			
			mInputs.emplace_back(parameterFloat);
		}


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
		}
	}

	void FlexBlockSequencePlayerComponentInstance::play()
	{
		mCurrentSequenceIndex = 0;
		mIsPlaying = true;
		mTime = 0.0;
	}
}