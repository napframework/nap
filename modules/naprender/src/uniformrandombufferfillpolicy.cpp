/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "uniformrandombufferfillpolicy.h"
#include "uniform.h"

// External Includes
#include <nap/core.h>


RTTI_BEGIN_CLASS(nap::UniformRandomStructBufferFillPolicy)
	RTTI_PROPERTY("LowerBound", &nap::UniformRandomStructBufferFillPolicy::mLowerBound, nap::rtti::EPropertyMetaData::Embedded | nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomStructBufferFillPolicy::mUpperBound, nap::rtti::EPropertyMetaData::Embedded | nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// UniformRandomBufferFillPolicy
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS(nap::UniformRandomIntBufferFillPolicy)
	RTTI_PROPERTY("LowerBound", &nap::UniformRandomIntBufferFillPolicy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomIntBufferFillPolicy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformRandomFloatBufferFillPolicy)
	RTTI_PROPERTY("LowerBound", &nap::UniformRandomFloatBufferFillPolicy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomFloatBufferFillPolicy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformRandomVec2BufferFillPolicy)
	RTTI_PROPERTY("LowerBound", &nap::UniformRandomVec2BufferFillPolicy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomVec2BufferFillPolicy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformRandomVec3BufferFillPolicy)
	RTTI_PROPERTY("LowerBound", &nap::UniformRandomVec3BufferFillPolicy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomVec3BufferFillPolicy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformRandomVec4BufferFillPolicy)
	RTTI_PROPERTY("LowerBound", &nap::UniformRandomVec4BufferFillPolicy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomVec4BufferFillPolicy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformRandomMat4BufferFillPolicy)
	RTTI_PROPERTY("LowerBound", &nap::UniformRandomMat4BufferFillPolicy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomMat4BufferFillPolicy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


namespace nap
{
	bool UniformRandomStructBufferFillPolicy::init(utility::ErrorState& errorState)
	{
		// int
		registerFillPolicyFunction(RTTI_OF(int), [](const UniformValue* uniform, const UniformValue* lowerBoundUniform, const UniformValue* upperBoundUniform, uint8* data)
		{
			const UniformInt* uniform_lowerbound_resolved = rtti_cast<const UniformInt>(lowerBoundUniform);
			const UniformInt* uniform_upperbound_resolved = rtti_cast<const UniformInt>(upperBoundUniform);

			assert(uniform_lowerbound_resolved != nullptr);
			assert(uniform_upperbound_resolved != nullptr);

			int value = math::random(uniform_lowerbound_resolved->mValue, uniform_upperbound_resolved->mValue);
			std::memcpy(data, &value, sizeof(int));
		});

		// float
		registerFillPolicyFunction(RTTI_OF(float), [](const UniformValue* uniform, const UniformValue* lowerBoundUniform, const UniformValue* upperBoundUniform, uint8* data)
		{
			const UniformFloat* uniform_lowerbound_resolved = rtti_cast<const UniformFloat>(lowerBoundUniform);
			const UniformFloat* uniform_upperbound_resolved = rtti_cast<const UniformFloat>(upperBoundUniform);

			assert(uniform_lowerbound_resolved != nullptr);
			assert(uniform_upperbound_resolved != nullptr);

			float value = math::random(uniform_lowerbound_resolved->mValue, uniform_upperbound_resolved->mValue);
			std::memcpy(data, &value, sizeof(float));
		});

		// vec2
		registerFillPolicyFunction(RTTI_OF(glm::vec2), [](const UniformValue* uniform, const UniformValue* lowerBoundUniform, const UniformValue* upperBoundUniform, uint8* data)
		{
			const UniformVec2* uniform_lowerbound_resolved = rtti_cast<const UniformVec2>(lowerBoundUniform);
			const UniformVec2* uniform_upperbound_resolved = rtti_cast<const UniformVec2>(upperBoundUniform);

			assert(uniform_lowerbound_resolved != nullptr);
			assert(uniform_upperbound_resolved != nullptr);

			glm::vec2 value = math::random(uniform_lowerbound_resolved->mValue, uniform_upperbound_resolved->mValue);
			std::memcpy(data, &value, sizeof(glm::vec2));
		});

		// vec3
		registerFillPolicyFunction(RTTI_OF(glm::vec3), [](const UniformValue* uniform, const UniformValue* lowerBoundUniform, const UniformValue* upperBoundUniform, uint8* data)
		{
			const UniformVec3* uniform_lowerbound_resolved = rtti_cast<const UniformVec3>(lowerBoundUniform);
			const UniformVec3* uniform_upperbound_resolved = rtti_cast<const UniformVec3>(upperBoundUniform);

			assert(uniform_lowerbound_resolved != nullptr);
			assert(uniform_upperbound_resolved != nullptr);

			glm::vec3 value = math::random(uniform_lowerbound_resolved->mValue, uniform_upperbound_resolved->mValue);
			std::memcpy(data, &value, sizeof(glm::vec3));
		});

		// vec4
		registerFillPolicyFunction(RTTI_OF(glm::vec4), [](const UniformValue* uniform, const UniformValue* lowerBoundUniform, const UniformValue* upperBoundUniform, uint8* data)
		{
			const UniformVec4* uniform_lowerbound_resolved = rtti_cast<const UniformVec4>(lowerBoundUniform);
			const UniformVec4* uniform_upperbound_resolved = rtti_cast<const UniformVec4>(upperBoundUniform);

			assert(uniform_lowerbound_resolved != nullptr);
			assert(uniform_upperbound_resolved != nullptr);

			glm::vec4 value = math::random(uniform_lowerbound_resolved->mValue, uniform_upperbound_resolved->mValue);
			std::memcpy(data, &value, sizeof(glm::vec4));
		});

		// mat4
		registerFillPolicyFunction(RTTI_OF(glm::mat4), [](const UniformValue* uniform, const UniformValue* lowerBoundUniform, const UniformValue* upperBoundUniform, uint8* data)
		{
			const UniformMat4* uniform_lowerbound_resolved = rtti_cast<const UniformMat4>(lowerBoundUniform);
			const UniformMat4* uniform_upperbound_resolved = rtti_cast<const UniformMat4>(upperBoundUniform);

			assert(uniform_lowerbound_resolved != nullptr);
			assert(uniform_upperbound_resolved != nullptr);

			glm::mat4 value = math::random(uniform_lowerbound_resolved->mValue, uniform_upperbound_resolved->mValue);
			std::memcpy(data, &value, sizeof(glm::mat4));
		});

		return true;
	}
}
