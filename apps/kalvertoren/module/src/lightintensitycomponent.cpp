#include "lightintensitycomponent.h"

// External Includes
#include <entity.h>
#include <mathutils.h>

// nap::lightintensitycomponent run time class definition 
RTTI_BEGIN_CLASS(nap::LightIntensityComponent)
	RTTI_PROPERTY("Sensors",			&nap::LightIntensityComponent::mLuxSensors,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SensorInfluence",	&nap::LightIntensityComponent::mSensorInfluence,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MasterIntensity",	&nap::LightIntensityComponent::mMasterIntensity,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("LuxInput",			&nap::LightIntensityComponent::mLuxRange,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("LightOutput",		&nap::LightIntensityComponent::mLightRange,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("LuxCurve",			&nap::LightIntensityComponent::mLuxPower,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SmoothTime",			&nap::LightIntensityComponent::mSmoothTime,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::lightintensitycomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LightIntensityComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void LightIntensityComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool LightIntensityComponentInstance::init(utility::ErrorState& errorState)
	{
		LightIntensityComponent* resource = getComponent<LightIntensityComponent>();

		// Make sure the light range is valid
		if (!errorState.check(resource->mLightRange.x < resource->mLightRange.y, "min output light exceeds max output light in range: %s", this->mID.c_str()))
			return false;

		if (!errorState.check(resource->mLightRange.x >= 0.0f && resource->mLightRange.y <= 1.0f, "light output is not within normalized range: %s", this->mID.c_str()))
			return false;

		for (auto& sensor : resource->mLuxSensors)
			mSensors.emplace_back(sensor.get());

		setMasterBrightness(resource->mMasterIntensity);
		setSensorInfluence(resource->mSensorInfluence);
		setLuxRange(resource->mLuxRange);
		setLightRange(resource->mLightRange);
		setLuxPower(resource->mLuxPower);
		setSmoothTime(resource->mSmoothTime);

		return true;
	}


	void LightIntensityComponentInstance::update(double deltaTime)
	{
		float seaverage = 0.0f;
		int sensor_reads = 0;

		// Read sensor data
		for (const auto& sensor : mSensors)
		{
			if (sensor->isOnline())
			{
				seaverage += sensor->getValue();
				sensor_reads++;
			}
		}

		// Check if there's a valid sensor value
		// If not we take the average weight of the min / max output light value
		// Otherwise we correctly map it
		float sensor_value = ((mLightRange.y - mLightRange.x) * 0.5f) + mLightRange.x;
		mLuxAverage = -1.0f;
		if (sensor_reads > 0)
		{
			mLuxAverage = seaverage / static_cast<float>(sensor_reads);
			float normalized_lux = math::fit<float>(mLuxAverage, mLuxRange.x, mLuxRange.y, 0.0f, 1.0f);
			normalized_lux  = math::power<float>(normalized_lux, mLuxPower);
			sensor_value = math::fit<float>(normalized_lux, 0.0f, 1.0f, mLightRange.x, mLightRange.y);
		}

		// Apply influence over final brightness
		sensor_value = math::lerp<float>(1.0f, sensor_value, mSensorInfluence);

		// Set brightness, ensure within 0-1 range
		float target_value = math::clamp<float>(mMasterIntensity * sensor_value, 0.0f, 1.0f);

		// Blend and store
		mBrightness = mIntensitySmoother.update(target_value, deltaTime);
	}


	void LightIntensityComponentInstance::setMasterBrightness(float value)
	{
		mMasterIntensity = math::clamp<float>(value, 0.0f, 1.0f);
	}


	void LightIntensityComponentInstance::setLuxRange(const glm::vec2 range)
	{
		mLuxRange.x = math::max<float>(range.x, 0.0f);
		mLuxRange.y = math::max<float>(range.y, 0.0f);
	}


	void LightIntensityComponentInstance::setLightRange(const glm::vec2 range)
	{
		mLightRange.x = math::clamp<float>(range.x, 0.0f, 1.0f);
		mLightRange.y = math::clamp<float>(range.y, 0.0f, 1.0f);
	}


	void LightIntensityComponentInstance::setSensorInfluence(float value)
	{
		mSensorInfluence = math::clamp<float>(value, 0.0f, 1.0f);
	}


	void LightIntensityComponentInstance::setSmoothTime(float value)
	{
		mIntensitySmoother.mSmoothTime = math::max<float>(value, 0.0f);
	}


	const std::vector<YoctoLuxSensor*>& LightIntensityComponentInstance::getSensors()
	{
		return mSensors;
	}

}
