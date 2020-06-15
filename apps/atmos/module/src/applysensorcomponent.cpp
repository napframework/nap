#include "applysensorcomponent.h"

// External Includes
#include <entity.h>
#include <mathutils.h>

RTTI_BEGIN_ENUM(nap::ApplySensorComponent::SensorCalcTypes)
	RTTI_ENUM_VALUE(nap::ApplySensorComponent::SensorCalcTypes::AVERAGE_VALUE, "Average"),
	RTTI_ENUM_VALUE(nap::ApplySensorComponent::SensorCalcTypes::CUMULATIVE_VALUE, "Cumulative"),
	RTTI_ENUM_VALUE(nap::ApplySensorComponent::SensorCalcTypes::CHOOSE_MAXIMUM_VALUE, "Maximum"),
	RTTI_ENUM_VALUE(nap::ApplySensorComponent::SensorCalcTypes::CHOOSE_MINIMUM_VALUE, "Minimum")
RTTI_END_ENUM

// nap::applysensorcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::ApplySensorComponent)
	RTTI_PROPERTY("Enabled",			&nap::ApplySensorComponent::mEnabled,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Sensors",			&nap::ApplySensorComponent::mSensors,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Parameters",			&nap::ApplySensorComponent::mParameters,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("InputRange",			&nap::ApplySensorComponent::mInputRange,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OutputRange",		&nap::ApplySensorComponent::mOutputRange,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Index",				&nap::ApplySensorComponent::mSelection,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SmoothTime",			&nap::ApplySensorComponent::mSmoothTime,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Sensor Output Type", &nap::ApplySensorComponent::mSensorCalcType,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Sensor Max Value",	&nap::ApplySensorComponent::mMaxSensorValue,	nap::rtti::EPropertyMetaData::Default)
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
		for (auto sensor : resource->mSensors)
		{
			mSensors.emplace_back(sensor.get());
		}
		mEnabled = resource->mEnabled.get();
		mInputRange = resource->mInputRange.get();
		mOutputRange = resource->mOutputRange.get();
		mEnabled = resource->mEnabled.get();
		mSmoothTime = resource->mSmoothTime.get();
		mCalcType = resource->mSensorCalcType;
		mMaxSensorValue = resource->mMaxSensorValue;

		switch (mCalcType)
		{
		case ApplySensorComponent::SensorCalcTypes::AVERAGE_VALUE:
			mCalcFunc = &ApplySensorComponentInstance::calcValueAverage;
			break;
		case ApplySensorComponent::SensorCalcTypes::CUMULATIVE_VALUE:
			mCalcFunc = &ApplySensorComponentInstance::calcValueCumulative;
			break;
		case ApplySensorComponent::SensorCalcTypes::CHOOSE_MAXIMUM_VALUE:
			mCalcFunc = &ApplySensorComponentInstance::calcValueMax;
			break;
		case ApplySensorComponent::SensorCalcTypes::CHOOSE_MINIMUM_VALUE:
			mCalcFunc = &ApplySensorComponentInstance::calcValueMin;
			break;
		default:
			assert(false); // some enum value is not implemented!
			return false;
			break;
		}

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

		if (!(mEnabled->mValue))
			return;

		for (auto* sensor : mSensors)
		{
			if (!sensor->isOnline())
				return;
		}	

		//
		float svalue = (float)(*this.*mCalcFunc)();

		// Get parameter to set and sensor value
		ParameterFloat* pfloat = static_cast<ParameterFloat*>(mCurrentParameter);

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


	double ApplySensorComponentInstance::calcValueAverage()
	{
		double average = 0.0;
		for (auto* sensor : mSensors)
		{
			double sensorValue = sensor->getValue();
			sensorValue = math::min<double>(sensorValue, mMaxSensorValue);
			average += sensorValue;
		}
		average /= mSensors.size();
		return average;
	}


	double ApplySensorComponentInstance::calcValueCumulative()
	{
		double value = 0.0;
		for (auto* sensor : mSensors)
		{
			double sensorValue = sensor->getValue();
			sensorValue = math::min<double>(sensorValue, mMaxSensorValue);
			value += sensorValue;
		}
		return value;
	}


	double ApplySensorComponentInstance::calcValueMax()
	{
		double max = math::min<double>();
		for (auto* sensor : mSensors)
		{
			double sensorValue = sensor->getValue();
			sensorValue = math::min<double>(sensorValue, mMaxSensorValue);
			if (sensorValue > max)
			{
				max = sensorValue;
			}
		}
		return max;
	}


	double ApplySensorComponentInstance::calcValueMin()
	{
		double min = math::max<double>();
		for (auto* sensor : mSensors)
		{
			double sensorValue = sensor->getValue();
			sensorValue = math::min<double>(sensorValue, mMaxSensorValue);
			if (sensorValue < min)
			{
				min = sensorValue;
			}
		}
		return min;
	}
}