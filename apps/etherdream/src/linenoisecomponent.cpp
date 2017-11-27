// Local Includes
#include "linenoisecomponent.h"

// External Includes
#include <entity.h>
#include <glm/gtc/noise.hpp>
#include <nap/logger.h>

RTTI_BEGIN_CLASS(nap::NoiseProperties)
	RTTI_PROPERTY("Frequency",				&nap::NoiseProperties::mFrequency,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FrequencySmoothTime",	&nap::NoiseProperties::mFrequencySmoothTime,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Speed",					&nap::NoiseProperties::mSpeed,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SpeedSmoothTime",		&nap::NoiseProperties::mSpeedSmoothTime,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Offset",					&nap::NoiseProperties::mOffset,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("OffsetSmoothTime",		&nap::NoiseProperties::mOffsetSmoothTime,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Amplitude",				&nap::NoiseProperties::mAmplitude,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AmplitudeSmoothTime",	&nap::NoiseProperties::mAmplitudeSmoothTime,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::LineNoiseComponent)
	RTTI_PROPERTY("Properties",			&nap::LineNoiseComponent::mProperties,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("BlendComponent",		&nap::LineNoiseComponent::mBlendComponent,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("TraceComponent",		&nap::LineNoiseComponent::mTraceComponent,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LineNoiseComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	
	bool LineNoiseComponentInstance::init(utility::ErrorState& errorState)
	{
		// Copy over properties and link to blend component
		mProperties = getComponent<LineNoiseComponent>()->mProperties;

		// Offset of noise component is always based on offset of trace component
		mProperties.mOffset = fmod(mTraceComponent->mProperties.mOffset + 0.1f, 1.0f);

		// Set smooth timing values
		mAmpSmoother.mSmoothTime = mProperties.mAmplitudeSmoothTime;
		mAmpSmoother.setValue(mProperties.mAmplitude);

		mFreqSmoother.mSmoothTime = mProperties.mFrequencySmoothTime;
		mFreqSmoother.setValue(mProperties.mFrequency);

		mOffsetSmoother.mSmoothTime = mProperties.mOffsetSmoothTime;
		mOffsetSmoother.setValue(mProperties.mOffset);

		mSpeedSmoother.mSmoothTime = mProperties.mSpeedSmoothTime;
		mSpeedSmoother.setValue(mProperties.mSpeed);

		return true;
	}


	void LineNoiseComponentInstance::update(double deltaTime)
	{
		// Update smoothers
		mSpeedSmoother.update(mProperties.mSpeed, deltaTime);
		mFreqSmoother.update(mProperties.mFrequency, deltaTime);
		mAmpSmoother.update(mProperties.mAmplitude, deltaTime);
		mOffsetSmoother.update(mProperties.mOffset, deltaTime);

		nap::PolyLine& line = mBlendComponent->getLine();

		// Update current time
		mCurrentTime += (deltaTime * mSpeedSmoother.getValue());

		// Get the normals and vertices to manipulate
		std::vector<glm::vec3>& normals = line.getNormalAttr().getData();
		std::vector<glm::vec3>& vertices = line.getPositionAttr().getData();
		std::vector<glm::vec3>& uvs = line.getUvAttr().getData();

		int vert_count = line.getMeshInstance().getNumVertices();

		for (int i = 0; i < vert_count; i++)
		{
			glm::vec2 current_uv = glm::vec2(uvs[i].x, uvs[i].y);
			current_uv.x = (current_uv.x * mFreqSmoother.getValue()) + (mCurrentTime + mOffsetSmoother.getValue());
			current_uv.y = (current_uv.y * mFreqSmoother.getValue()) + (mCurrentTime + mOffsetSmoother.getValue());

			float v = glm::simplex(current_uv) * mAmpSmoother.getValue();

			vertices[i] += (normals[i] * v);
		}

		utility::ErrorState error;
		if (!line.getMeshInstance().update(error))
		{
			nap::Logger::warn(error.toString().c_str());
		}

	}
}