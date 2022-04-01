/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "structfillpolicy.h"
#include "uniform.h"

// External Includes
#include <nap/core.h>

RTTI_BEGIN_CLASS(nap::StructFillPolicy)
	RTTI_PROPERTY("FillPolicies", &nap::StructFillPolicy::mFillPolicies, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StructFillPolicy::BaseEntry)
	RTTI_PROPERTY("Name", &nap::StructFillPolicy::BaseEntry::mName, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::StructFillPolicyUInt)
	RTTI_PROPERTY("FillPolicy", &nap::StructFillPolicyUInt::mFillPolicy, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::StructFillPolicyInt)
	RTTI_PROPERTY("FillPolicy", &nap::StructFillPolicyInt::mFillPolicy, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::StructFillPolicyFloat)
	RTTI_PROPERTY("FillPolicy", &nap::StructFillPolicyFloat::mFillPolicy, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::StructFillPolicyVec2)
	RTTI_PROPERTY("FillPolicy", &nap::StructFillPolicyVec2::mFillPolicy, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::StructFillPolicyVec3)
	RTTI_PROPERTY("FillPolicy", &nap::StructFillPolicyVec3::mFillPolicy, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::StructFillPolicyVec4)
	RTTI_PROPERTY("FillPolicy", &nap::StructFillPolicyVec4::mFillPolicy, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::StructFillPolicyMat4)
	RTTI_PROPERTY("FillPolicy", &nap::StructFillPolicyMat4::mFillPolicy, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static functions
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Defaults to memset zero if policy is a nullptr
	 */
	template<typename T>
	static bool setDataFromPolicy(const FillPolicy<T>* policy, uint8* data, int count)
	{
		if (policy != nullptr)
		{
			policy->fill(count, reinterpret_cast<T*>(data));
			return true;
		}
		std::memset(data, 0, sizeof(T) * count);
		return false;
	}


	bool StructFillPolicy::fill(StructBufferDescriptor* descriptor, uint8* data, utility::ErrorState& errorState)
	{
		// Fill the buffer
		size_t element_size = 0;
		for (size_t idx = 0; idx < descriptor->mCount; idx++)
		{
			if (!fillFromUniformRecursive(descriptor->mElement.get(), data + element_size * idx, element_size, errorState))
			{
				errorState.fail(utility::stringFormat("Failed to fill specified data storage", this->mID.c_str()));
				return false;
			}
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// StructFillPolicy
	//////////////////////////////////////////////////////////////////////////

	bool StructFillPolicy::fillFromUniformRecursive(const UniformStruct* uniformStruct, uint8* data, size_t& outElementSize, utility::ErrorState& errorState)
	{
		size_t size = 0;
		for (int idx = 0; idx < uniformStruct->mUniforms.size(); idx++)
		{
			const auto& uniform = uniformStruct->mUniforms[idx];
			rtti::TypeInfo uniform_type = uniformStruct->mUniforms[idx]->get_type();

			if (uniform_type.is_derived_from(RTTI_OF(UniformStructArray)))
			{
				const UniformStructArray* uniform_resolved = rtti_cast<const UniformStructArray>(uniform.get());
				if (!uniform_resolved->mStructs.empty())
				{
					for (int struct_idx = 0; struct_idx < uniformStruct->mUniforms.size(); struct_idx++)
					{
						size_t struct_element_size;
						if (!fillFromUniformRecursive(uniform_resolved->mStructs[struct_idx].get(), data + size, struct_element_size, errorState))
							return false;

						size += struct_element_size;
					}
				}
			}
			else if (uniform_type.is_derived_from(RTTI_OF(UniformValueArray)))
			{
				const UniformValueArray* uniform_resolved = rtti_cast<const UniformValueArray>(uniform.get());

				if (uniform_type == RTTI_OF(TypedUniformValueArray<uint>))
				{
					const auto* policy = findPolicy<uint>(uniform_resolved->mName);
					setDataFromPolicy(policy, data + size, uniform_resolved->getCount());
					size += sizeof(uint) * uniform_resolved->getCount();
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<int>))
				{
					const auto* policy = findPolicy<int>(uniform_resolved->mName);
					setDataFromPolicy(policy, data + size, uniform_resolved->getCount());
					size += sizeof(int) * uniform_resolved->getCount();
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<float>))
				{
					const auto* policy = findPolicy<float>(uniform_resolved->mName);
					setDataFromPolicy(policy, data + size, uniform_resolved->getCount());
					size += sizeof(float) * uniform_resolved->getCount();
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<glm::vec2>))
				{
					const auto* policy = findPolicy<glm::vec2>(uniform_resolved->mName);
					setDataFromPolicy(policy, data + size, uniform_resolved->getCount());
					size += sizeof(glm::vec2) * uniform_resolved->getCount();
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<glm::vec3>))
				{
					const auto* policy = findPolicy<glm::vec3>(uniform_resolved->mName);
					setDataFromPolicy(policy, data + size, uniform_resolved->getCount());
					size += sizeof(glm::vec3) * uniform_resolved->getCount();
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<glm::vec4>))
				{
					const auto* policy = findPolicy<glm::vec4>(uniform_resolved->mName);
					setDataFromPolicy(policy, data + size, uniform_resolved->getCount());
					size += sizeof(glm::vec4) * uniform_resolved->getCount();
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<glm::mat4>))
				{
					const auto* policy = findPolicy<glm::mat4>(uniform_resolved->mName);
					setDataFromPolicy(policy, data + size, uniform_resolved->getCount());
					size += sizeof(glm::mat4) * uniform_resolved->getCount();
				}
				else
				{
					errorState.fail("Unsupported uniform value type");
					return false;
				}
			}
			else if (uniform_type.is_derived_from(RTTI_OF(UniformStruct)))
			{
				const UniformStruct* uniform_resolved = rtti_cast<UniformStruct>(uniform.get());
				size_t struct_element_size;
				if (!fillFromUniformRecursive(uniform_resolved, data + size, struct_element_size, errorState))
					return false;

				size += struct_element_size;
			}
			else if (uniform_type.is_derived_from(RTTI_OF(UniformValue)))
			{
				const UniformValue* uniform_resolved = rtti_cast<UniformValue>(uniform.get());

				if (uniform_type == RTTI_OF(TypedUniformValue<uint>))
				{
					const auto* policy = findPolicy<uint>(uniform_resolved->mName);
					setDataFromPolicy(policy, data + size, 1);
					size += sizeof(uint);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<int>))
				{
					const auto* policy = findPolicy<int>(uniform_resolved->mName);
					setDataFromPolicy(policy, data + size, 1);
					size += sizeof(int);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<float>))
				{
					const auto* policy = findPolicy<float>(uniform_resolved->mName);
					setDataFromPolicy(policy, data + size, 1);
					size += sizeof(float);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<glm::vec2>))
				{
					const auto* policy = findPolicy<glm::vec2>(uniform_resolved->mName);
					setDataFromPolicy(policy, data + size, 1);
					size += sizeof(glm::vec2);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<glm::vec3>))
				{
					const auto* policy = findPolicy<glm::vec3>(uniform_resolved->mName);
					setDataFromPolicy(policy, data + size, 1);
					size += sizeof(glm::vec3);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<glm::vec4>))
				{
					const auto* policy = findPolicy<glm::vec4>(uniform_resolved->mName);
					setDataFromPolicy(policy, data + size, 1);
					size += sizeof(glm::vec4);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<glm::mat4>))
				{
					const auto* policy = findPolicy<glm::mat4>(uniform_resolved->mName);
					setDataFromPolicy(policy, data + size, 1);
					size += sizeof(glm::mat4);
				}
				else
				{
					errorState.fail("Unsupported uniform value type");
					return false;
				}
			}
		}
		outElementSize = size;
		return true;
	}
}
