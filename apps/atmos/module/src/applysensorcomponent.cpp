#include "applysensorcomponent.h"

// External Includes
#include <entity.h>
#include <mathutils.h>

// nap::applysensorcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::ApplySensorComponent)
	RTTI_PROPERTY("Sensor",		&nap::ApplySensorComponent::mSensor,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Parameters", &nap::ApplySensorComponent::mParameters,	nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Index",		&nap::ApplySensorComponent::mSelection,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::applysensorcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ApplySensorComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void ApplySensorComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool ApplySensorComponentInstance::init(utility::ErrorState& errorState)
	{
		// Copy parameters
		ApplySensorComponent* resource = getComponent<ApplySensorComponent>();
		mParameters.reserve(resource->mParameters.size());
		for (auto& parameter : resource->mParameters)
			mParameters.emplace_back(parameter.get());

		// Copy data
		mSensor = resource->mSensor.get();
		mEnabled = resource->mEnabled;

		// Connect value changed and select parameter
		resource->mSelection->valueChanged.connect(mIndexChangedSlot);
		selectParameter(resource->mSelection->mValue);
		return true;
	}


	void ApplySensorComponentInstance::update(double deltaTime)
	{
		// No parameter to set
		if (mCurrentParameter == nullptr)
			return;

		// Only apply mapped value when on-line
		if (mSensor->isOnline() && mEnabled)
		{
			mCurrentParameter->setValue(mSensor->getValue());
		}
	}


	void ApplySensorComponentInstance::selectParameter(int index)
	{
		// Don't do anything when there are no parameters to map
		if (mParameters.empty())
		{
			mCurrentParameter = nullptr;
			return;
		}

		// Clamp current index and select
		mCurrentIndex = math::clamp<int>(index, 0, mParameters.size() - 1);
		mCurrentParameter = mParameters[mCurrentIndex];
	}
}