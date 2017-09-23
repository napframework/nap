// Local Includes
#include "linenoisecomponent.h"

// External Includes
#include <nap/entity.h>
#include <glm/gtc/noise.hpp>
#include <nap/logger.h>

RTTI_BEGIN_CLASS(nap::NoiseProperties)
	RTTI_PROPERTY("Frequency",			&nap::NoiseProperties::mFrequency,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Speed",				&nap::NoiseProperties::mSpeed,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Offset",				&nap::NoiseProperties::mOffset,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Amplitude",			&nap::NoiseProperties::mAmplitude,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::LineNoiseComponent)
	RTTI_PROPERTY("Properties",			&nap::LineNoiseComponent::mProperties,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("BlendComponent",		&nap::LineNoiseComponent::mBlendComponent,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LineNoiseComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	
	bool LineNoiseComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// Copy over properties and link to blend component
		mBlendComponent = getComponent<LineNoiseComponent>()->mBlendComponent.get();
		mProperties = getComponent<LineNoiseComponent>()->mProperties;

		return true;
	}


	void LineNoiseComponentInstance::update(double deltaTime)
	{
		nap::PolyLine& line = mBlendComponent->getLine();

		// Update current time
		mCurrentTime += (deltaTime * mProperties.mSpeed);

		// Get the normals and vertices to manipulate
		std::vector<glm::vec3>& normals = line.getNormalAttr().getData();
		std::vector<glm::vec3>& vertices = line.getPositionAttr().getData();
		std::vector<glm::vec3>& uvs = line.getUvAttr().getData();

		int vert_count = line.getMeshInstance().getNumVertices();

		for (int i = 0; i < vert_count; i++)
		{
			glm::vec2 current_uv = glm::vec2(uvs[i].x, uvs[i].y);
			current_uv.x = (current_uv.x * mProperties.mFrequency) + (mCurrentTime + mProperties.mOffset);
			current_uv.y = (current_uv.y * mProperties.mFrequency) + (mCurrentTime + mProperties.mOffset);

			float v = glm::simplex(current_uv) * mProperties.mAmplitude;

			vertices[i] += (normals[i] * v);
		}

		utility::ErrorState error;
		if (!line.getMeshInstance().update(error))
		{
			nap::Logger::warn(error.toString().c_str());
		}

	}
}