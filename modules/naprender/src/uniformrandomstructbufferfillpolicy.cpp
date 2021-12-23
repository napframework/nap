/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "uniformrandomstructbufferfillpolicy.h"
#include "uniform.h"

// External Includes
#include <nap/core.h>

RTTI_BEGIN_CLASS(nap::UniformRandomStructBufferFillPolicy)
	RTTI_PROPERTY("LowerBound", &nap::UniformRandomStructBufferFillPolicy::mLowerBound, nap::rtti::EPropertyMetaData::Embedded | nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomStructBufferFillPolicy::mUpperBound, nap::rtti::EPropertyMetaData::Embedded | nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


namespace nap
{
	////////////////////////////////////////////////////////////
	// Static
	////////////////////////////////////////////////////////////

	static const std::vector<rtti::TypeInfo> sSupportedValueTypes =
	{
		RTTI_OF(int),
		RTTI_OF(float),
		RTTI_OF(glm::vec2),
		RTTI_OF(glm::vec3),
		RTTI_OF(glm::vec4),
		RTTI_OF(glm::mat4)
	};


	////////////////////////////////////////////////////////////
	// UniformRandomStructBufferFillPolicy
	////////////////////////////////////////////////////////////

	bool UniformRandomStructBufferFillPolicy::init(utility::ErrorState& errorState)
	{
		// int
		registerFillPolicyFunction(RTTI_OF(int), [](const UniformValue* uniform, const UniformValue* referenceUniformA, const UniformValue* referenceUniformB, uint8* data)
		{
			const UniformInt* uniform_lowerbound_resolved = rtti_cast<const UniformInt>(referenceUniformA);
			const UniformInt* uniform_upperbound_resolved = rtti_cast<const UniformInt>(referenceUniformB);

			assert(uniform_lowerbound_resolved != nullptr);
			assert(uniform_upperbound_resolved != nullptr);

			int value = (uniform_lowerbound_resolved->mValue != uniform_upperbound_resolved->mValue) ? math::random(uniform_lowerbound_resolved->mValue, uniform_upperbound_resolved->mValue) : uniform_lowerbound_resolved->mValue;
			std::memcpy(data, &value, sizeof(int));
		});

		// float
		registerFillPolicyFunction(RTTI_OF(float), [](const UniformValue* uniform, const UniformValue* referenceUniformA, const UniformValue* referenceUniformB, uint8* data)
		{
			const UniformFloat* uniform_lowerbound_resolved = rtti_cast<const UniformFloat>(referenceUniformA);
			const UniformFloat* uniform_upperbound_resolved = rtti_cast<const UniformFloat>(referenceUniformB);

			assert(uniform_lowerbound_resolved != nullptr);
			assert(uniform_upperbound_resolved != nullptr);

			float value = (uniform_lowerbound_resolved->mValue != uniform_upperbound_resolved->mValue) ? math::random(uniform_lowerbound_resolved->mValue, uniform_upperbound_resolved->mValue) : uniform_lowerbound_resolved->mValue;
			std::memcpy(data, &value, sizeof(float));
		});

		// vec2
		registerFillPolicyFunction(RTTI_OF(glm::vec2), [](const UniformValue* uniform, const UniformValue* referenceUniformA, const UniformValue* referenceUniformB, uint8* data)
		{
			const UniformVec2* uniform_lowerbound_resolved = rtti_cast<const UniformVec2>(referenceUniformA);
			const UniformVec2* uniform_upperbound_resolved = rtti_cast<const UniformVec2>(referenceUniformB);

			assert(uniform_lowerbound_resolved != nullptr);
			assert(uniform_upperbound_resolved != nullptr);

			glm::vec2 value = (uniform_lowerbound_resolved->mValue != uniform_upperbound_resolved->mValue) ? math::random(uniform_lowerbound_resolved->mValue, uniform_upperbound_resolved->mValue) : uniform_lowerbound_resolved->mValue;
			std::memcpy(data, &value, sizeof(glm::vec2));
		});

		// vec3
		registerFillPolicyFunction(RTTI_OF(glm::vec3), [](const UniformValue* uniform, const UniformValue* referenceUniformA, const UniformValue* referenceUniformB, uint8* data)
		{
			const UniformVec3* uniform_lowerbound_resolved = rtti_cast<const UniformVec3>(referenceUniformA);
			const UniformVec3* uniform_upperbound_resolved = rtti_cast<const UniformVec3>(referenceUniformB);

			assert(uniform_lowerbound_resolved != nullptr);
			assert(uniform_upperbound_resolved != nullptr);

			glm::vec3 value = (uniform_lowerbound_resolved->mValue != uniform_upperbound_resolved->mValue) ? math::random(uniform_lowerbound_resolved->mValue, uniform_upperbound_resolved->mValue) : uniform_lowerbound_resolved->mValue;
			std::memcpy(data, &value, sizeof(glm::vec3));
		});

		// vec4
		registerFillPolicyFunction(RTTI_OF(glm::vec4), [](const UniformValue* uniform, const UniformValue* referenceUniformA, const UniformValue* referenceUniformB, uint8* data)
		{
			const UniformVec4* uniform_lowerbound_resolved = rtti_cast<const UniformVec4>(referenceUniformA);
			const UniformVec4* uniform_upperbound_resolved = rtti_cast<const UniformVec4>(referenceUniformB);

			assert(uniform_lowerbound_resolved != nullptr);
			assert(uniform_upperbound_resolved != nullptr);

			glm::vec4 value = (uniform_lowerbound_resolved->mValue != uniform_upperbound_resolved->mValue) ? math::random(uniform_lowerbound_resolved->mValue, uniform_upperbound_resolved->mValue) : uniform_lowerbound_resolved->mValue;
			std::memcpy(data, &value, sizeof(glm::vec4));
		});

		// mat4
		registerFillPolicyFunction(RTTI_OF(glm::mat4), [](const UniformValue* uniform, const UniformValue* referenceUniformA, const UniformValue* referenceUniformB, uint8* data)
		{
			const UniformMat4* uniform_lowerbound_resolved = rtti_cast<const UniformMat4>(referenceUniformA);
			const UniformMat4* uniform_upperbound_resolved = rtti_cast<const UniformMat4>(referenceUniformB);

			assert(uniform_lowerbound_resolved != nullptr);
			assert(uniform_upperbound_resolved != nullptr);

			glm::mat4 value = (uniform_lowerbound_resolved->mValue != uniform_upperbound_resolved->mValue) ? math::random(uniform_lowerbound_resolved->mValue, uniform_upperbound_resolved->mValue) : uniform_lowerbound_resolved->mValue;
			std::memcpy(data, &value, sizeof(glm::mat4));
		});

		return true;
	}


	bool UniformRandomStructBufferFillPolicy::fill(StructBufferDescriptor* descriptor, uint8* data, utility::ErrorState& errorState)
	{
		// Verify the function map
		for (const auto& type : sSupportedValueTypes)
		{
			if (errorState.check(mFillValueFunctionMap.find(type) == mFillValueFunctionMap.end(), utility::stringFormat("Missing fill function implementation for type '%s' in function map", type.get_name().to_string().c_str())))
				return false;
		}

		// Fill the buffer
		size_t element_size = 0;
		for (size_t idx = 0; idx < descriptor->mCount; idx++)
			element_size = fillFromUniformRecursive(descriptor->mElement.get(), mLowerBound.get(), mUpperBound.get(), data + element_size * idx);

		return true;
	}
}
