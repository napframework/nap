/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "linenoisecomponent.h"

// External Includes
#include <entity.h>
#include <glm/gtc/noise.hpp>
#include <nap/logger.h>

RTTI_BEGIN_STRUCT(nap::NoiseProperties)
	RTTI_PROPERTY("Frequency",				&nap::NoiseProperties::mFrequency,				nap::rtti::EPropertyMetaData::Default,	"Noise frequency")
	RTTI_PROPERTY("FrequencySmoothTime",	&nap::NoiseProperties::mFrequencySmoothTime,	nap::rtti::EPropertyMetaData::Default,	"Noise frequency smooth time (seconds)")
	RTTI_PROPERTY("Speed",					&nap::NoiseProperties::mSpeed,					nap::rtti::EPropertyMetaData::Default,	"Noise speed")
	RTTI_PROPERTY("SpeedSmoothTime",		&nap::NoiseProperties::mSpeedSmoothTime,		nap::rtti::EPropertyMetaData::Default,	"Noise speed smooth time (seconds)")
	RTTI_PROPERTY("Offset",					&nap::NoiseProperties::mOffset,					nap::rtti::EPropertyMetaData::Default,	"Noise offset")
	RTTI_PROPERTY("OffsetSmoothTime",		&nap::NoiseProperties::mOffsetSmoothTime,		nap::rtti::EPropertyMetaData::Default,	"Noise offset smooth time (seconds)")
	RTTI_PROPERTY("Amplitude",				&nap::NoiseProperties::mAmplitude,				nap::rtti::EPropertyMetaData::Default,	"Noise amplitude")
	RTTI_PROPERTY("AmplitudeSmoothTime",	&nap::NoiseProperties::mAmplitudeSmoothTime,	nap::rtti::EPropertyMetaData::Default,	"Noise ampliture smooth time (seconds)")
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(nap::LineNoiseComponent, "Applies displacement to the blended poly line")
	RTTI_PROPERTY("Properties",			&nap::LineNoiseComponent::mProperties,			nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("BlendComponent",		&nap::LineNoiseComponent::mBlendComponent,		nap::rtti::EPropertyMetaData::Required)
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

		// Set smooth timing values
		mAmpSmoother.mSmoothTime = mProperties.mAmplitudeSmoothTime;
		mAmpSmoother.setValue(mProperties.mAmplitude->mValue);

		mFreqSmoother.mSmoothTime = mProperties.mFrequencySmoothTime;
		mFreqSmoother.setValue(mProperties.mFrequency->mValue);

		mOffsetSmoother.mSmoothTime = mProperties.mOffsetSmoothTime;
		mOffsetSmoother.setValue(mProperties.mOffset->mValue);

		mSpeedSmoother.mSmoothTime = mProperties.mSpeedSmoothTime;
		mSpeedSmoother.setValue(mProperties.mSpeed->mValue);

		return true;
	}


	void LineNoiseComponentInstance::update(double deltaTime)
	{
		// Update smoothers
		mSpeedSmoother.update(mProperties.mSpeed->mValue, deltaTime);
		mFreqSmoother.update(mProperties.mFrequency->mValue, deltaTime);
		mAmpSmoother.update(mProperties.mAmplitude->mValue, deltaTime);
		mOffsetSmoother.update(mProperties.mOffset->mValue, deltaTime);

		nap::PolyLine& line = mBlendComponent->getLine();

		// Update current time
		mCurrentTime += (deltaTime * mSpeedSmoother.getValue());

		// Get the normals and vertices to manipulate
		std::vector<glm::vec3>& normals = line.getNormalAttr().getData();
		std::vector<glm::vec3>& vertices = line.getPositionAttr().getData();
		std::vector<glm::vec3>& uvs = line.getUvAttr().getData();

		int vert_count = line.getMeshInstance().getNumVertices();

		// Apply noise based on normal
		for (int i = 0; i < vert_count; i++)
		{
			glm::vec2 current_uv = glm::vec2(uvs[i].x, uvs[i].y);
			current_uv.x = (current_uv.x * mFreqSmoother.getValue()) + (mCurrentTime + mOffsetSmoother.getValue());
			current_uv.y = (current_uv.y * mFreqSmoother.getValue()) + (mCurrentTime + mOffsetSmoother.getValue());
			float v = glm::simplex(current_uv) * mAmpSmoother.getValue();
			vertices[i] += (normals[i] * v);
		}

		// Update normal based on displaced vertices
		updateNormals(normals, vertices);

		// Push changes to the gpu
		utility::ErrorState error;
		if (!line.getMeshInstance().update(error))
		{
			nap::Logger::warn(error.toString().c_str());
		}
	}


	void LineNoiseComponentInstance::updateNormals(std::vector<glm::vec3>& normals, const std::vector<glm::vec3>& vertices)
	{
		glm::vec3 crossn(0.0f, 0.0f, -1.0f);
		for (int i = 1; i < vertices.size() - 1; i++)
		{
			// Get vector pointing to next and previous vertex
			glm::vec3 dnormal_one = glm::normalize(vertices[i + 1] - vertices[i]);
			glm::vec3 dnormal_two = glm::normalize(vertices[i] - vertices[i - 1]);

			// Rotate around z using cross product
			normals[i] = glm::cross(glm::normalize(math::lerp<glm::vec3>(dnormal_one, dnormal_two, 0.5f)), crossn);
		}
	
		// Fix beginning and end
		normals[0] = glm::cross(glm::normalize(vertices[1] - vertices.front()), crossn);
		normals.back() = normals[normals.size() - 2];
	}
}
