/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "uniform.h"

namespace nap
{
	static void getUniformStructDepthRecursive(const UniformStruct& uniformStruct, int& outMax, int depth = 0)
	{
		outMax = math::max(outMax, depth);
		for (const auto& uniform : uniformStruct.mUniforms)
		{
			rtti::TypeInfo uniform_type = uniform->get_type();

			if (uniform_type.is_derived_from(RTTI_OF(UniformStructArray)))
			{
				UniformStructArray* uniform_resolved = rtti_cast<UniformStructArray>(uniform.get());
				if (!uniform_resolved->mStructs.empty())
					getUniformStructDepthRecursive(*uniform_resolved->mStructs[0].get(), outMax, depth + 1);
			}
			else if (uniform_type.is_derived_from(RTTI_OF(UniformStruct)))
			{
				UniformStruct* uniform_resolved = rtti_cast<UniformStruct>(uniform.get());
				getUniformStructDepthRecursive(*uniform_resolved, outMax, depth + 1);
			}
		}
	}


	int NAPAPI getUniformStructDepth(const UniformStruct& uniformStruct)
	{
		int max = 0;
		getUniformStructDepthRecursive(uniformStruct, max);
		return max;
	}


	size_t NAPAPI getUniformStructSizeRecursive(const UniformStruct& uniformStruct)
	{
		size_t size = 0;
		for (const auto& uniform : uniformStruct.mUniforms)
		{
			rtti::TypeInfo uniform_type = uniform->get_type();

			if (uniform_type.is_derived_from(RTTI_OF(UniformStructArray)))
			{
				UniformStructArray* uniform_resolved = rtti_cast<UniformStructArray>(uniform.get());
				if (!uniform_resolved->mStructs.empty())
				{
					size_t struct_element_size = getUniformStructSizeRecursive(*uniform_resolved->mStructs[0].get());
					size += struct_element_size * uniform_resolved->mStructs.size();
				}
			}
			else if (uniform_type.is_derived_from(RTTI_OF(UniformValueArray)))
			{
				UniformValueArray* uniform_resolved = rtti_cast<UniformValueArray>(uniform.get());

				if (uniform_type == RTTI_OF(TypedUniformValueArray<int>))
					size += sizeof(int) * uniform_resolved->getCount();

				else if (uniform_type == RTTI_OF(TypedUniformValueArray<float>))
					size += sizeof(float) * uniform_resolved->getCount();

				else if (uniform_type == RTTI_OF(TypedUniformValueArray<glm::vec2>))
					size += sizeof(glm::vec2) * uniform_resolved->getCount();

				else if (uniform_type == RTTI_OF(TypedUniformValueArray<glm::vec3>))
					size += sizeof(glm::vec3) * uniform_resolved->getCount();

				else if (uniform_type == RTTI_OF(TypedUniformValueArray<glm::vec4>))
					size += sizeof(glm::vec4) * uniform_resolved->getCount();

				else if (uniform_type == RTTI_OF(TypedUniformValueArray<glm::mat4>))
					size += sizeof(glm::mat4) * uniform_resolved->getCount();
			}
			else if (uniform_type.is_derived_from(RTTI_OF(UniformStruct)))
			{
				UniformStruct* uniform_resolved = rtti_cast<UniformStruct>(uniform.get());
				size += getUniformStructSizeRecursive(*uniform_resolved);
			}
			else if (uniform_type.is_derived_from(RTTI_OF(UniformValue)))
			{
				if (uniform_type == RTTI_OF(TypedUniformValue<int>))
					size += sizeof(int);

				else if (uniform_type == RTTI_OF(TypedUniformValue<float>))
					size += sizeof(float);

				else if (uniform_type == RTTI_OF(TypedUniformValue<glm::vec2>))
					size += sizeof(glm::vec2);

				else if (uniform_type == RTTI_OF(TypedUniformValue<glm::vec3>))
					size += sizeof(glm::vec3);

				else if (uniform_type == RTTI_OF(TypedUniformValue<glm::vec4>))
					size += sizeof(glm::vec4);

				else if (uniform_type == RTTI_OF(TypedUniformValue<glm::mat4>))
					size += sizeof(glm::mat4);
			}
		}
		return size;
	}
}
