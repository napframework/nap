// Local Includes
#include "linemodulationcomponent.h"

// External Includes
#include <nap/entity.h>
#include <mathutils.h>

RTTI_BEGIN_CLASS(nap::ModulationProperties)
	RTTI_PROPERTY("Frequency",			&nap::ModulationProperties::mFrequency,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Speed",				&nap::ModulationProperties::mSpeed,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Offset",				&nap::ModulationProperties::mOffset,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Amplitude",			&nap::ModulationProperties::mAmplitude,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Waveform",			&nap::ModulationProperties::mWaveform,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Normalize",			&nap::ModulationProperties::mNormalize,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::LineModulationComponent)
	RTTI_PROPERTY("Properties",			&nap::LineModulationComponent::mProperties,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("BlendComponent",		&nap::LineModulationComponent::mBlendComponent, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LineModulationComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{

	bool LineModulationComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// Copy properties
		mProperties = getComponent<LineModulationComponent>()->mProperties;

		// Get the blend component we want to apply the modulation on to
		mBlendComponent = getComponent<LineModulationComponent>()->mBlendComponent.get();

		return true;
	}


	void LineModulationComponentInstance::update(double deltaTime)
	{
		mCurrentTime -= (deltaTime * mProperties.mSpeed);

		// Fetch spline
		PolyLine& spline = mBlendComponent->getLine();

		// Get distance of current line
		std::map<float, int> distance_map;
		float length = spline.getDistances(distance_map);

		// Get the normals and vertices to manipulate
		const std::vector<glm::vec3>& normals = spline.getNormalAttr().getData();
		std::vector<glm::vec3>& vertices = spline.getPositionAttr().getData();

		// Calculate offset value based on time and personal offset
		float offset = mProperties.mNormalize ? mProperties.mOffset : length * mProperties.mOffset;
		offset = offset + mCurrentTime;

		for (auto& dist : distance_map)
		{
			// Get time (t) value for wave form. If the frequencies are normalized a frequency of 1 
			// means 1 modulation over the entire spline, no matter how long the line is. Otherwise
			// modulation frequency is based on the length of the line
			float t = mProperties.mNormalize ? (dist.first / length) + offset : dist.first + offset;
			float wave_value = math::waveform(mProperties.mWaveform, t, mProperties.mFrequency);
			wave_value = math::fit<float>(wave_value, 0.0f, 1.0f, -1.0f, 1.0f) * mProperties.mAmplitude;
			vertices[dist.second] += (normals[dist.second] * wave_value);
		}
	}

}