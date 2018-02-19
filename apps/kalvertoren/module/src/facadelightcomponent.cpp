#include "facadelightcomponent.h"

// External Includes
#include <entity.h>
#include <mathutils.h>

// nap::facadelightcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::FacadeLightComponent)
RTTI_PROPERTY("Controller", &nap::FacadeLightComponent::mArtnetController,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MinChannel", &nap::FacadeLightComponent::mMinChannel,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaxChannel", &nap::FacadeLightComponent::mMaxChannel,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Send",		&nap::FacadeLightComponent::mSend,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::facadelightcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FacadeLightComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void FacadeLightComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool FacadeLightComponentInstance::init(utility::ErrorState& errorState)
	{
		mController = getComponent<FacadeLightComponent>()->mArtnetController.get();
		mMinChannel = getComponent<FacadeLightComponent>()->mMinChannel;
		mMaxChannel = getComponent<FacadeLightComponent>()->mMaxChannel;
		mSend = getComponent<FacadeLightComponent>()->mSend;

		if (!errorState.check(mMinChannel <= mMaxChannel, "invalid channel range: %s", this->mID.c_str()))
			return false;

		// Populate data buffer
		int count = (mMaxChannel - mMinChannel) + 1;
		mData = std::vector<uint8_t>(count, math::max<uint8_t>());
		return true;
	}


	void FacadeLightComponentInstance::update(double deltaTime)
	{
		if (!mSend) { return; }
		mController->send(mData, mMinChannel);
	}
}