/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "bufferfillpolicy.h"
#include "uniform.h"

// External Includes
#include <nap/core.h>

RTTI_DEFINE_BASE(nap::BaseFillPolicy)
RTTI_DEFINE_BASE(nap::BaseValueBufferFillPolicy)

RTTI_DEFINE_BASE(nap::IntBufferFillPolicy)
RTTI_DEFINE_BASE(nap::FloatBufferFillPolicy)
RTTI_DEFINE_BASE(nap::Vec2BufferFillPolicy)
RTTI_DEFINE_BASE(nap::Vec3BufferFillPolicy)
RTTI_DEFINE_BASE(nap::Vec4BufferFillPolicy)
RTTI_DEFINE_BASE(nap::Mat4BufferFillPolicy)


//////////////////////////////////////////////////////////////////////////
// UniformRandomBufferFillPolicy
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS(nap::UniformRandomFillPolicy)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// ConstantBufferFillPolicy
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS(nap::ConstantIntBufferFillPolicy)
	RTTI_PROPERTY("Constant", &nap::ConstantIntBufferFillPolicy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantFloatBufferFillPolicy)
	RTTI_PROPERTY("Constant", &nap::ConstantFloatBufferFillPolicy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantIntBufferFillPolicy)
	RTTI_PROPERTY("Constant", &nap::ConstantIntBufferFillPolicy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantVec2BufferFillPolicy)
	RTTI_PROPERTY("Constant", &nap::ConstantVec2BufferFillPolicy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantVec3BufferFillPolicy)
	RTTI_PROPERTY("Constant", &nap::ConstantVec3BufferFillPolicy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantVec4BufferFillPolicy)
	RTTI_PROPERTY("Constant", &nap::ConstantVec4BufferFillPolicy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantMat4BufferFillPolicy)
	RTTI_PROPERTY("Constant", &nap::ConstantMat4BufferFillPolicy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


namespace nap
{
	////////////////////////////////////////////////////////////
	// UniformRandomFillPolicy
	////////////////////////////////////////////////////////////

	bool UniformRandomFillPolicy::init(utility::ErrorState& errorState)
	{
		// int
		registerFillPolicyFunction(RTTI_OF(int), [](const Uniform* lowerBoundUniform, const Uniform* upperBoundUniform, uint8* data)
		{
			const UniformInt* uniform_lowerbound_resolved = rtti_cast<const UniformInt>(lowerBoundUniform);
			const UniformInt* uniform_upperbound_resolved = rtti_cast<const UniformInt>(upperBoundUniform);

			assert(uniform_lowerbound_resolved != nullptr);
			assert(uniform_upperbound_resolved != nullptr);

			int value = math::random(uniform_lowerbound_resolved->mValue, uniform_upperbound_resolved->mValue);
			std::memcpy(data, &value, sizeof(value));
		});

		// float
		registerFillPolicyFunction(RTTI_OF(float), [](const Uniform* lowerBoundUniform, const Uniform* upperBoundUniform, uint8* data)
		{
			const UniformFloat* uniform_lowerbound_resolved = rtti_cast<const UniformFloat>(lowerBoundUniform);
			const UniformFloat* uniform_upperbound_resolved = rtti_cast<const UniformFloat>(upperBoundUniform);

			assert(uniform_lowerbound_resolved != nullptr);
			assert(uniform_upperbound_resolved != nullptr);

			float value = math::random(uniform_lowerbound_resolved->mValue, uniform_upperbound_resolved->mValue);
			std::memcpy(data, &value, sizeof(value));
		});

		// vec2
		registerFillPolicyFunction(RTTI_OF(glm::vec2), [](const Uniform* lowerBoundUniform, const Uniform* upperBoundUniform, uint8* data)
		{
			const UniformVec2* uniform_lowerbound_resolved = rtti_cast<const UniformVec2>(lowerBoundUniform);
			const UniformVec2* uniform_upperbound_resolved = rtti_cast<const UniformVec2>(upperBoundUniform);

			assert(uniform_lowerbound_resolved != nullptr);
			assert(uniform_upperbound_resolved != nullptr);

			glm::vec2 value = math::random(uniform_lowerbound_resolved->mValue, uniform_upperbound_resolved->mValue);
			std::memcpy(data, &value, sizeof(value));
		});

		// vec3
		registerFillPolicyFunction(RTTI_OF(glm::vec3), [](const Uniform* lowerBoundUniform, const Uniform* upperBoundUniform, uint8* data)
		{
			const UniformVec3* uniform_lowerbound_resolved = rtti_cast<const UniformVec3>(lowerBoundUniform);
			const UniformVec3* uniform_upperbound_resolved = rtti_cast<const UniformVec3>(upperBoundUniform);

			assert(uniform_lowerbound_resolved != nullptr);
			assert(uniform_upperbound_resolved != nullptr);

			glm::vec3 value = math::random(uniform_lowerbound_resolved->mValue, uniform_upperbound_resolved->mValue);
			std::memcpy(data, &value, sizeof(value));
		});

		// vec4
		registerFillPolicyFunction(RTTI_OF(glm::vec4), [](const Uniform* lowerBoundUniform, const Uniform* upperBoundUniform, uint8* data)
		{
			const UniformVec4* uniform_lowerbound_resolved = rtti_cast<const UniformVec4>(lowerBoundUniform);
			const UniformVec4* uniform_upperbound_resolved = rtti_cast<const UniformVec4>(upperBoundUniform);

			assert(uniform_lowerbound_resolved != nullptr);
			assert(uniform_upperbound_resolved != nullptr);

			glm::vec4 value = math::random(uniform_lowerbound_resolved->mValue, uniform_upperbound_resolved->mValue);
			std::memcpy(data, &value, sizeof(value));
		});

		// mat4
		registerFillPolicyFunction(RTTI_OF(glm::mat4), [](const Uniform* lowerBoundUniform, const Uniform* upperBoundUniform, uint8* data)
		{
			const UniformMat4* uniform_lowerbound_resolved = rtti_cast<const UniformMat4>(lowerBoundUniform);
			const UniformMat4* uniform_upperbound_resolved = rtti_cast<const UniformMat4>(upperBoundUniform);

			assert(uniform_lowerbound_resolved != nullptr);
			assert(uniform_upperbound_resolved != nullptr);

			glm::mat4 value = math::random(uniform_lowerbound_resolved->mValue, uniform_upperbound_resolved->mValue);
			std::memcpy(data, &value, sizeof(value));
		});

		return true;
	}


	////////////////////////////////////////////////////////////
	// BaseFillPolicy
	////////////////////////////////////////////////////////////

	bool BaseFillPolicy::init(utility::ErrorState& errorState)
	{
		return true;
	}


	bool BaseFillPolicy::registerFillPolicyFunction(rtti::TypeInfo type, FillPolicyFunction fillFunction)
	{
		auto it = mFillMap.find(type);
		if (it != mFillMap.end())
		{
			assert(false);
			return false;
		}
		mFillMap.emplace(type, fillFunction);
		return true;
	}


	int BaseFillPolicy::fillFromUniformRecursive(const UniformStruct* uniformStruct, uint8* data)
	{
		size_t size = 0;
		for (const auto& uniform : uniformStruct->mUniforms)
		{
			rtti::TypeInfo uniform_type = uniform->get_type();

			if (uniform_type.is_derived_from(RTTI_OF(UniformStructArray)))
			{
				UniformStructArray* uniform_resolved = rtti_cast<UniformStructArray>(uniform.get());
				if (!uniform_resolved->mStructs.empty())
				{
					assert(uniform_resolved->mStructs.size() == 1);
					size_t struct_element_size = fillFromUniformRecursive(uniform_resolved->mStructs[0].get(), data + size);
					size += struct_element_size * uniform_resolved->mStructs.size();
				}
			}
			else if (uniform_type.is_derived_from(RTTI_OF(UniformValueArray)))
			{
				UniformValueArray* uniform_resolved = rtti_cast<UniformValueArray>(uniform.get());

				if (uniform_type == RTTI_OF(TypedUniformValueArray<int>))
				{
					setValues<Uniform, int>(uniform_resolved, uniform_resolved, uniform_resolved->getCount(), data + size);
					size += sizeof(int) * uniform_resolved->getCount();
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<float>))
				{
					setValues<Uniform, float>(uniform_resolved, uniform_resolved, uniform_resolved->getCount(), data + size);
					size += sizeof(float) * uniform_resolved->getCount();
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<glm::vec2>))
				{
					setValues<Uniform, glm::vec2>(uniform_resolved, uniform_resolved, uniform_resolved->getCount(), data + size);
					size += sizeof(glm::vec2) * uniform_resolved->getCount();
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<glm::vec3>))
				{
					setValues<Uniform, glm::vec3>(uniform_resolved, uniform_resolved, uniform_resolved->getCount(), data + size);
					size += sizeof(glm::vec3) * uniform_resolved->getCount();
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<glm::vec4>))
				{
					setValues<Uniform, glm::vec4>(uniform_resolved, uniform_resolved, uniform_resolved->getCount(), data + size);
					size += sizeof(glm::vec4) * uniform_resolved->getCount();
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<glm::mat4>))
				{
					setValues<Uniform, glm::mat4>(uniform_resolved, uniform_resolved, uniform_resolved->getCount(), data + size);
					size += sizeof(glm::mat4) * uniform_resolved->getCount();
				}
				else
				{
					assert(false);
				}
			}
			else if (uniform_type.is_derived_from(RTTI_OF(UniformStruct)))
			{
				UniformStruct* uniform_resolved = rtti_cast<UniformStruct>(uniform.get());
				size_t struct_element_size = fillFromUniformRecursive(uniform_resolved, data + size);
				size += struct_element_size;
			}
			else if (uniform_type.is_derived_from(RTTI_OF(UniformValue)))
			{
				UniformValue* uniform_resolved = rtti_cast<UniformValue>(uniform.get());

				if (uniform_type == RTTI_OF(TypedUniformValue<int>))
				{
					setValues<Uniform, int>(uniform_resolved, uniform_resolved, 1, data + size);
					size += sizeof(int);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<float>))
				{
					setValues<Uniform, float>(uniform_resolved, uniform_resolved, 1, data + size);
					size += sizeof(float);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<glm::vec2>))
				{
					setValues<Uniform, glm::vec2>(uniform_resolved, uniform_resolved, 1, data + size);
					size += sizeof(glm::vec2);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<glm::vec3>))
				{
					setValues<Uniform, glm::vec3>(uniform_resolved, uniform_resolved, 1, data + size);
					size += sizeof(glm::vec3);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<glm::vec4>))
				{
					setValues<Uniform, glm::vec4>(uniform_resolved, uniform_resolved, 1, data + size);
					size += sizeof(glm::vec4);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<glm::mat4>))
				{
					setValues<Uniform, glm::mat4>(uniform_resolved, uniform_resolved, 1, data + size);
					size += sizeof(glm::mat4);
				}
				else
				{
					assert(false);
				}
			}
		}
		return size;
	}


	bool BaseFillPolicy::fill(StructBufferDescriptor* descriptor, uint8* data, utility::ErrorState& errorState)
	{
		size_t element_size = 0;
		for (size_t i = 0; i < descriptor->mCount; i++)
			fillFromUniformRecursive(descriptor->mElement.get(), data);

		return true;
	}
}
