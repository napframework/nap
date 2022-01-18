/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "structbufferfillpolicy.h"
#include "uniform.h"

// External Includes
#include <nap/core.h>

RTTI_BEGIN_CLASS(nap::StructBufferFillPolicy)
	RTTI_PROPERTY("VariableFillPolicies", &nap::StructBufferFillPolicy::mVariableFillPolicies, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BaseShaderVariableFillPolicyEntry)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::IntFillPolicyEntry)
	RTTI_PROPERTY("Name", &nap::IntFillPolicyEntry::mName, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("ValueFillPolicy", &nap::IntFillPolicyEntry::mValueFillPolicy, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::FloatFillPolicyEntry)
	RTTI_PROPERTY("Name", &nap::FloatFillPolicyEntry::mName, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("ValueFillPolicy", &nap::FloatFillPolicyEntry::mValueFillPolicy, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Vec2FillPolicyEntry)
	RTTI_PROPERTY("Name", &nap::Vec2FillPolicyEntry::mName, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("ValueFillPolicy", &nap::Vec2FillPolicyEntry::mValueFillPolicy, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Vec3FillPolicyEntry)
	RTTI_PROPERTY("Name", &nap::Vec3FillPolicyEntry::mName, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("ValueFillPolicy", &nap::Vec3FillPolicyEntry::mValueFillPolicy, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Vec4FillPolicyEntry)
	RTTI_PROPERTY("Name", &nap::Vec4FillPolicyEntry::mName, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("ValueFillPolicy", &nap::Vec4FillPolicyEntry::mValueFillPolicy, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Mat4FillPolicyEntry)
	RTTI_PROPERTY("Name", &nap::Mat4FillPolicyEntry::mName, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("ValueFillPolicy", &nap::Mat4FillPolicyEntry::mValueFillPolicy, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// StructBufferFillPolicy
	//////////////////////////////////////////////////////////////////////////

	bool StructBufferFillPolicy::fillFromUniformRecursive(const UniformStruct* uniformStruct, uint8* data, size_t& outElementSize, utility::ErrorState& errorState)
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
						size_t struct_element_size = fillFromUniformRecursive(uniform_resolved->mStructs[struct_idx].get(), data + size, outElementSize, errorState);
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
					if (!errorState.check(policy != nullptr, utility::stringFormat("Failed to find shader variable name '%s' registered in VariableFillPolicies", uniform_resolved->mName.c_str()).c_str()))
						return false;

					policy->fill(uniform_resolved->getCount(), data + size);
					size += sizeof(uint) * uniform_resolved->getCount();
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<int>))
				{
					const auto* policy = findPolicy<int>(uniform_resolved->mName);
					if (!errorState.check(policy != nullptr, utility::stringFormat("Failed to find shader variable name '%s' registered in VariableFillPolicies", uniform_resolved->mName.c_str()).c_str()))
						return false;

					policy->fill(uniform_resolved->getCount(), data + size);
					size += sizeof(int) * uniform_resolved->getCount();
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<float>))
				{
					const auto* policy = findPolicy<float>(uniform_resolved->mName);
					if (!errorState.check(policy != nullptr, utility::stringFormat("Failed to find shader variable name '%s' registered in VariableFillPolicies", uniform_resolved->mName.c_str()).c_str()))
						return false;

					policy->fill(uniform_resolved->getCount(), data + size);
					size += sizeof(float) * uniform_resolved->getCount();
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<glm::vec2>))
				{
					const auto* policy = findPolicy<glm::vec2>(uniform_resolved->mName);
					if (!errorState.check(policy != nullptr, utility::stringFormat("Failed to find shader variable name '%s' registered in VariableFillPolicies", uniform_resolved->mName.c_str()).c_str()))
						return false;

					policy->fill(uniform_resolved->getCount(), data + size);
					size += sizeof(glm::vec2) * uniform_resolved->getCount();
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<glm::vec3>))
				{
					const auto* policy = findPolicy<glm::vec3>(uniform_resolved->mName);
					if (!errorState.check(policy != nullptr, utility::stringFormat("Failed to find shader variable name '%s' registered in VariableFillPolicies", uniform_resolved->mName.c_str()).c_str()))
						return false;

					policy->fill(uniform_resolved->getCount(), data + size);
					size += sizeof(glm::vec3) * uniform_resolved->getCount();
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<glm::vec4>))
				{
					const auto* policy = findPolicy<glm::vec4>(uniform_resolved->mName);
					if (!errorState.check(policy != nullptr, utility::stringFormat("Failed to find shader variable name '%s' registered in VariableFillPolicies", uniform_resolved->mName.c_str()).c_str()))
						return false;

					policy->fill(uniform_resolved->getCount(), data + size);
					size += sizeof(glm::vec4) * uniform_resolved->getCount();
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<glm::mat4>))
				{
					const auto* policy = findPolicy<glm::mat4>(uniform_resolved->mName);
					if (!errorState.check(policy != nullptr, utility::stringFormat("Failed to find shader variable name '%s' registered in VariableFillPolicies", uniform_resolved->mName.c_str()).c_str()))
						return false;

					policy->fill(uniform_resolved->getCount(), data + size);
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
				size_t struct_element_size = fillFromUniformRecursive(uniform_resolved, data + size, outElementSize, errorState);
				size += struct_element_size;
			}
			else if (uniform_type.is_derived_from(RTTI_OF(UniformValue)))
			{
				const UniformValue* uniform_resolved = rtti_cast<UniformValue>(uniform.get());

				if (uniform_type == RTTI_OF(TypedUniformValue<uint>))
				{
					const auto* policy = findPolicy<uint>(uniform_resolved->mName);
					if (!errorState.check(policy != nullptr, utility::stringFormat("Failed to find shader variable name '%s' registered in VariableFillPolicies", uniform_resolved->mName.c_str()).c_str()))
						return false;

					policy->fill(1, data + size);
					size += sizeof(uint);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<int>))
				{
					const auto* policy = findPolicy<int>(uniform_resolved->mName);
					if (!errorState.check(policy != nullptr, utility::stringFormat("Failed to find shader variable name '%s' registered in VariableFillPolicies", uniform_resolved->mName.c_str()).c_str()))
						return false;

					policy->fill(1, data + size);
					size += sizeof(int);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<float>))
				{
					const auto* policy = findPolicy<float>(uniform_resolved->mName);
					if (!errorState.check(policy != nullptr, utility::stringFormat("Failed to find shader variable name '%s' registered in VariableFillPolicies", uniform_resolved->mName.c_str()).c_str()))
						return false;

					policy->fill(1, data + size);
					size += sizeof(float);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<glm::vec2>))
				{
					const auto* policy = findPolicy<glm::vec2>(uniform_resolved->mName);
					if (!errorState.check(policy != nullptr, utility::stringFormat("Failed to find shader variable name '%s' registered in VariableFillPolicies", uniform_resolved->mName.c_str()).c_str()))
						return false;

					policy->fill(1, data + size);
					size += sizeof(glm::vec2);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<glm::vec3>))
				{
					const auto* policy = findPolicy<glm::vec3>(uniform_resolved->mName);
					if (!errorState.check(policy != nullptr, utility::stringFormat("Failed to find shader variable name '%s' registered in VariableFillPolicies", uniform_resolved->mName.c_str()).c_str()))
						return false;

					policy->fill(1, data + size);
					size += sizeof(glm::vec3);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<glm::vec4>))
				{
					const auto* policy = findPolicy<glm::vec4>(uniform_resolved->mName);
					if (!errorState.check(policy != nullptr, utility::stringFormat("Failed to find shader variable name '%s' registered in VariableFillPolicies", uniform_resolved->mName.c_str()).c_str()))
						return false;

					policy->fill(1, data + size);
					size += sizeof(glm::vec4);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<glm::mat4>))
				{
					const auto* policy = findPolicy<glm::mat4>(uniform_resolved->mName);
					if (!errorState.check(policy != nullptr, utility::stringFormat("Failed to find shader variable name '%s' registered in VariableFillPolicies", uniform_resolved->mName.c_str()).c_str()))
						return false;

					policy->fill(1, data + size);
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


	bool StructBufferFillPolicy::fill(StructBufferDescriptor* descriptor, uint8* data, utility::ErrorState& errorState)
	{
		// Fill the buffer
		size_t element_size = 0;
		for (size_t idx = 0; idx < descriptor->mCount; idx++)
			element_size = fillFromUniformRecursive(descriptor->mElement.get(), data + element_size * idx, element_size, errorState);

		return true;
	}
}
