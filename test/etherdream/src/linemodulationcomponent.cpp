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
		mCurrentTime += (deltaTime * mProperties.mSpeed);

		// Figure out final offset
		float offset = 1.0f - fmod(mCurrentTime + math::clamp<float>(mProperties.mOffset, 0.0f, 1.0f), 1.0f);

		// Fetch spline
		PolyLine& spline = mBlendComponent->getLine();
		int vert_count = spline.getMeshInstance().getNumVertices();

		// Get current frequency
		float curr_freq = mProperties.mFrequency;
		float curr_ampl = mProperties.mAmplitude;

		// Get the normals
		const std::vector<glm::vec3>& normals = spline.getNormalAttr().getData();
		std::vector<glm::vec3>& vertices = spline.getPositionAttr().getData();

		for (int i = 0; i < vert_count; i++)
		{
			// Get normal for point
			const glm::vec3& normal = normals[i];
			glm::vec3& vertex = vertices[i];

			// Get floating point index
			float sample_index = fmod((offset / curr_freq) + (static_cast<float>(i) / static_cast<float>(vert_count)), 1.0f);

			// Get data based on lfo type
			float wave_value = math::fit<float>(math::waveform(mProperties.mWaveform, sample_index, curr_freq), 0.0f, 1.0f, -1.0f, 1.0f) * curr_ampl;

			// Displace vert along normal
			vertex += (normal * wave_value);
		}
	}

}