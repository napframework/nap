#include "applysensorcomponent.h"

// External Includes
#include <entity.h>
#include <mathutils.h>

// nap::applysensorcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::ApplySensorComponent)
	RTTI_PROPERTY("Enabled",		&nap::ApplySensorComponent::mEnabled,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Sensor",			&nap::ApplySensorComponent::mSensor,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Parameters",		&nap::ApplySensorComponent::mParameters,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("InputRange",		&nap::ApplySensorComponent::mInputRange,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OutputRange",	&nap::ApplySensorComponent::mOutputRange,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Index",			&nap::ApplySensorComponent::mSelection,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SmoothTime",		&nap::ApplySensorComponent::mSmoothTime,	nap::rtti::EPropertyMetaData::Required)
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


	bool ApplySensorComponent::init(utility::ErrorState& errorState)
	{
		mSelection->mMinimum = 0;
		mSelection->mMaximum = mParameters.empty() ? 0 : mParameters.size() - 1;
		return true;
	}


	bool ApplySensorComponentInstance::init(utility::ErrorState& errorState)
	{
		// Copy parameters
		ApplySensorComponent* resource = getComponent<ApplySensorComponent>();
		mParameters.reserve(resource->mParameters.size());
		for (auto& parameter : resource->mParameters)
		{
			mParameters.emplace_back(parameter.get());
		}

		// Copy data and references
		mSensor = resource->mSensor.get();
		mEnabled = resource->mEnabled.get();
		mInputRange = resource->mInputRange.get();
		mOutputRange = resource->mOutputRange.get();
		mEnabled = resource->mEnabled.get();
		mSmoothTime = resource->mSmoothTime.get();

		// Update smoother value
		mSmoother.setValue(mInputRange->mValue.y);

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

		if (!mSensor->isOnline() || !(mEnabled->mValue))
			return;

		// Get parameter to set and sensor value
		ParameterFloat* pfloat = static_cast<ParameterFloat*>(mCurrentParameter);
		float svalue = (float)(mSensor->getValue());

		// Smoothly interpolate sensor value, clamp based on highest sensor input value
		float clamp_max = mInputRange->mValue.x > mInputRange->mValue.y ? mInputRange->mValue.x : mInputRange->mValue.y;
		svalue = math::min<float>(svalue, clamp_max);
		
		// Get smoothed value
		mSmoother.mSmoothTime = mSmoothTime->mValue;
		svalue = mSmoother.update(svalue, deltaTime);

		// Mapped
		float mapped = math::fit<float>(svalue, 
			mInputRange->mValue.x,  mInputRange->mValue.y, 
			mOutputRange->mValue.x, mOutputRange->mValue.y);

		// Set value
		pfloat->setValue(mapped);
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