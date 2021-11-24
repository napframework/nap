/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "bufferfillpolicy.h"
#include "uniform.h"

// External Includes
#include <nap/core.h>

RTTI_DEFINE_BASE(nap::BaseStructBufferFillPolicy)

RTTI_DEFINE_BASE(nap::BaseValueBufferFillPolicy)

RTTI_DEFINE_BASE(nap::IntBufferFillPolicy)
RTTI_DEFINE_BASE(nap::FloatBufferFillPolicy)
RTTI_DEFINE_BASE(nap::Vec2BufferFillPolicy)
RTTI_DEFINE_BASE(nap::Vec3BufferFillPolicy)
RTTI_DEFINE_BASE(nap::Vec4BufferFillPolicy)
RTTI_DEFINE_BASE(nap::Mat4BufferFillPolicy)


//////////////////////////////////////////////////////////////////////////
// ConstantBufferFillPolicy
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS(nap::ConstantStructBufferFillPolicy)
RTTI_END_CLASS

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
	// ConstantStructBufferFillPolicy
	////////////////////////////////////////////////////////////

	bool ConstantStructBufferFillPolicy::init(utility::ErrorState& errorState)
	{
		// int
		registerFillPolicyFunction(RTTI_OF(int), [](const UniformValue* uniform, const UniformValue* referenceUniformA, const UniformValue* referenceUniformB, uint8* data)
		{
			const UniformInt* uniform_resolved = rtti_cast<const UniformInt>(uniform);
			assert(uniform_resolved != nullptr);
			std::memcpy(data, &uniform_resolved->mValue, sizeof(int));
		});

		// float
		registerFillPolicyFunction(RTTI_OF(float), [](const UniformValue* uniform, const UniformValue* referenceUniformA, const UniformValue* referenceUniformB, uint8* data)
		{
			const UniformFloat* uniform_resolved = rtti_cast<const UniformFloat>(uniform);
			assert(uniform_resolved != nullptr);
			std::memcpy(data, &uniform_resolved->mValue, sizeof(float));
		});

		// vec2
		registerFillPolicyFunction(RTTI_OF(glm::vec2), [](const UniformValue* uniform, const UniformValue* referenceUniformA, const UniformValue* referenceUniformB, uint8* data)
		{
			const UniformVec2* uniform_resolved = rtti_cast<const UniformVec2>(uniform);
			assert(uniform_resolved != nullptr);
			std::memcpy(data, &uniform_resolved->mValue, sizeof(glm::vec2));
		});

		// vec3
		registerFillPolicyFunction(RTTI_OF(glm::vec3), [](const UniformValue* uniform, const UniformValue* referenceUniformA, const UniformValue* referenceUniformB, uint8* data)
		{
			const UniformVec3* uniform_resolved = rtti_cast<const UniformVec3>(uniform);
			assert(uniform_resolved != nullptr);
			std::memcpy(data, &uniform_resolved->mValue, sizeof(glm::vec3));
		});

		// vec4
		registerFillPolicyFunction(RTTI_OF(glm::vec4), [](const UniformValue* uniform, const UniformValue* referenceUniformA, const UniformValue* referenceUniformB, uint8* data)
		{
			const UniformVec4* uniform_resolved = rtti_cast<const UniformVec4>(uniform);
			assert(uniform_resolved != nullptr);
			std::memcpy(data, &uniform_resolved->mValue, sizeof(glm::vec4));
		});

		// mat4
		registerFillPolicyFunction(RTTI_OF(glm::mat4), [](const UniformValue* uniform, const UniformValue* referenceUniformA, const UniformValue* referenceUniformB, uint8* data)
		{
			const UniformMat4* uniform_resolved = rtti_cast<const UniformMat4>(uniform);
			assert(uniform_resolved != nullptr);
			std::memcpy(data, &uniform_resolved->mValue, sizeof(glm::mat4));
		});

		return true;
	}


	////////////////////////////////////////////////////////////
	// BaseStructBufferFillPolicy
	////////////////////////////////////////////////////////////

	bool BaseStructBufferFillPolicy::registerFillPolicyFunction(rtti::TypeInfo type, FillValueFunction fillFunction)
	{
		auto it = mFillValueFunctionMap.find(type);
		if (it != mFillValueFunctionMap.end())
		{
			assert(false);
			return false;
		}
		mFillValueFunctionMap.emplace(type, fillFunction);
		return true;
	}


	size_t BaseStructBufferFillPolicy::fillFromUniformRecursive(const UniformStruct* uniformStruct, const UniformStruct* referenceUniformStructA, const UniformStruct* referenceUniformStructB, uint8* data)
	{
		size_t size = 0;
		for (int idx = 0; idx < uniformStruct->mUniforms.size(); idx++)
		{
			const auto& uniform = uniformStruct->mUniforms[idx];
			rtti::TypeInfo uniform_type = uniformStruct->mUniforms[idx]->get_type();

			if (uniform_type.is_derived_from(RTTI_OF(UniformStructArray)))
			{
				const UniformStructArray* uniform_resolved = rtti_cast<const UniformStructArray>(uniform.get());
				const UniformStructArray* uniform_a_resolved = referenceUniformStructA != nullptr ? rtti_cast<const UniformStructArray>(referenceUniformStructA->findUniform(uniform_resolved->mName)) : nullptr;
				const UniformStructArray* uniform_b_resolved = referenceUniformStructB != nullptr ? rtti_cast<const UniformStructArray>(referenceUniformStructB->findUniform(uniform_resolved->mName)) : nullptr;

				if (!uniform_resolved->mStructs.empty())
				{
					for (int struct_idx = 0; struct_idx < uniformStruct->mUniforms.size(); struct_idx++)
					{
						size_t struct_element_size = fillFromUniformRecursive(
							uniform_resolved->mStructs[struct_idx].get(),
							uniform_a_resolved != nullptr ? uniform_a_resolved->mStructs[struct_idx].get() : nullptr,
							uniform_b_resolved != nullptr ? uniform_b_resolved->mStructs[struct_idx].get() : nullptr,
							data + size
						);
						size += struct_element_size;
					}
				}
			}
			else if (uniform_type.is_derived_from(RTTI_OF(UniformValueArray)))
			{
				const UniformValueArray* uniform_resolved = rtti_cast<const UniformValueArray>(uniform.get());
				const UniformValueArray* uniform_a_resolved = referenceUniformStructA != nullptr ? rtti_cast<const UniformValueArray>(referenceUniformStructA->findUniform(uniform_resolved->mName)) : nullptr;
				const UniformValueArray* uniform_b_resolved = referenceUniformStructB != nullptr ? rtti_cast<const UniformValueArray>(referenceUniformStructB->findUniform(uniform_resolved->mName)) : nullptr;

				if (uniform_type == RTTI_OF(TypedUniformValueArray<int>))
				{
					setValues<int>(uniform_resolved, uniform_a_resolved, uniform_b_resolved, uniform_resolved->getCount(), data + size);
					size += sizeof(int) * uniform_resolved->getCount();
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<float>))
				{
					setValues<float>(uniform_resolved, uniform_a_resolved, uniform_b_resolved, uniform_resolved->getCount(), data + size);
					size += sizeof(float) * uniform_resolved->getCount();
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<glm::vec2>))
				{
					setValues<glm::vec2>(uniform_resolved, uniform_a_resolved, uniform_b_resolved, uniform_resolved->getCount(), data + size);
					size += sizeof(glm::vec2) * uniform_resolved->getCount();
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<glm::vec3>))
				{
					setValues<glm::vec3>(uniform_resolved, uniform_a_resolved, uniform_b_resolved, uniform_resolved->getCount(), data + size);
					size += sizeof(glm::vec3) * uniform_resolved->getCount();
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<glm::vec4>))
				{
					setValues<glm::vec4>(uniform_resolved, uniform_a_resolved, uniform_b_resolved, uniform_resolved->getCount(), data + size);
					size += sizeof(glm::vec4) * uniform_resolved->getCount();
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<glm::mat4>))
				{
					setValues<glm::mat4>(uniform_resolved, uniform_a_resolved, uniform_b_resolved, uniform_resolved->getCount(), data + size);
					size += sizeof(glm::mat4) * uniform_resolved->getCount();
				}
				else
				{
					assert(false);
				}
			}
			else if (uniform_type.is_derived_from(RTTI_OF(UniformStruct)))
			{
				const UniformStruct* uniform_resolved = rtti_cast<UniformStruct>(uniform.get());
				const UniformStruct* uniform_a_resolved = referenceUniformStructA != nullptr ? rtti_cast<const UniformStruct>(referenceUniformStructA->findUniform(uniform_resolved->mName)) : nullptr;
				const UniformStruct* uniform_b_resolved = referenceUniformStructB != nullptr ? rtti_cast<const UniformStruct>(referenceUniformStructB->findUniform(uniform_resolved->mName)) : nullptr;

				size_t struct_element_size = fillFromUniformRecursive(uniform_resolved, uniform_a_resolved, uniform_b_resolved, data + size);
				size += struct_element_size;
			}
			else if (uniform_type.is_derived_from(RTTI_OF(UniformValue)))
			{
				const UniformValue* uniform_resolved = rtti_cast<UniformValue>(uniform.get());
				const UniformValue* uniform_a_resolved = referenceUniformStructA != nullptr ? rtti_cast<const UniformValue>(referenceUniformStructA->findUniform(uniform_resolved->mName)) : nullptr;
				const UniformValue* uniform_b_resolved = referenceUniformStructB != nullptr ? rtti_cast<const UniformValue>(referenceUniformStructB->findUniform(uniform_resolved->mName)) : nullptr;

				if (uniform_type == RTTI_OF(TypedUniformValue<int>))
				{
					setValues<int>(uniform_resolved, uniform_a_resolved, uniform_b_resolved, 1, data + size);
					size += sizeof(int);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<float>))
				{
					setValues<float>(uniform_resolved, uniform_a_resolved, uniform_b_resolved, 1, data + size);
					size += sizeof(float);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<glm::vec2>))
				{
					setValues<glm::vec2>(uniform_resolved, uniform_a_resolved, uniform_b_resolved, 1, data + size);
					size += sizeof(glm::vec2);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<glm::vec3>))
				{
					setValues<glm::vec3>(uniform_resolved, uniform_a_resolved, uniform_b_resolved, 1, data + size);
					size += sizeof(glm::vec3);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<glm::vec4>))
				{
					setValues<glm::vec4>(uniform_resolved, uniform_a_resolved, uniform_b_resolved, 1, data + size);
					size += sizeof(glm::vec4);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<glm::mat4>))
				{
					setValues<glm::mat4>(uniform_resolved, uniform_a_resolved, uniform_b_resolved, 1, data + size);
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


	bool BaseStructBufferFillPolicy::fill(StructBufferDescriptor* descriptor, uint8* data, utility::ErrorState& errorState)
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
			element_size = fillFromUniformRecursive(descriptor->mElement.get(), nullptr, nullptr, data + element_size * idx);

		return true;
	}
}
