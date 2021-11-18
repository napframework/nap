/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "gpustructbuffer.h"
#include "renderservice.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUStructBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Descriptor", &nap::GPUStructBuffer::mDescriptor, nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Usage", &nap::GPUStructBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::GPUStructBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS


namespace nap
{
	static void getUniformStructDepthRecursive(const UniformStruct& uniformStruct, int& max, int depth = 0)
	{
		max = math::max(max, depth);
		for (const auto& uniform : uniformStruct.mUniforms)
		{
			rtti::TypeInfo uniform_type = uniform->get_type();

			if (uniform_type.is_derived_from(RTTI_OF(UniformStructArray)))
			{
				UniformStructArray* uniform_resolved = rtti_cast<UniformStructArray>(uniform.get());
				if (!uniform_resolved->mStructs.empty())
					getUniformStructDepthRecursive(*uniform_resolved->mStructs[0].get(), max, depth+1);
			}
			else if (uniform_type.is_derived_from(RTTI_OF(UniformStruct)))
			{
				UniformStruct* uniform_resolved = rtti_cast<UniformStruct>(uniform.get());
				getUniformStructDepthRecursive(*uniform_resolved, max, depth+1);
			}
		}
	}


	static int getUniformStructDepth(const UniformStruct& uniformStruct)
	{
		int max = 0;
		getUniformStructDepthRecursive(uniformStruct, max);
		return max;
	}


	static int getUniformStructSizeRecursive(const UniformStruct& uniformStruct)
	{
		int size = 0;
		for (const auto& uniform : uniformStruct.mUniforms)
		{
			rtti::TypeInfo uniform_type = uniform->get_type();

			if (uniform_type.is_derived_from(RTTI_OF(UniformStructArray)))
			{
				UniformStructArray* uniform_resolved = rtti_cast<UniformStructArray>(uniform.get());
				if (!uniform_resolved->mStructs.empty())
				{
					int struct_element_size = getUniformStructSizeRecursive(*uniform_resolved->mStructs[0].get());
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


	bool GPUStructBuffer::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mDescriptor.mCount >= 0, "Descriptor.Count must be non-zero and non-negative"))
			return false;

		if (!GPUBuffer::init(errorState))
			return false;

		UniformStruct* element_descriptor = mDescriptor.mElement.get();

		// Verify maximum depth
		int depth = getUniformStructDepth(*element_descriptor);
		if (!errorState.check(depth == 0, "GPUStructBuffers with elements that exceed depth=1 are currently not supported"))
			return false;

		// Calculate element size in bytes
		mElementSize = getUniformStructSizeRecursive(*element_descriptor);
		size_t total_size = getSize();

		// Create a staging buffer to upload
		auto staging_buffer = std::make_unique<uint8[]>(total_size);
		if (mBufferFillPolicy != nullptr)
		{
			mBufferFillPolicy->fill(mDescriptor, staging_buffer.get(), errorState);
		}
		else
		{
			std::memset(staging_buffer.get(), 0, total_size);
		}

		// Prepare staging buffer upload
		return setDataInternal(staging_buffer.get(), total_size, static_cast<VkBufferUsageFlagBits>(getBufferUsage(EBufferObjectType::Storage)), errorState);
	}


	bool GPUStructBuffer::setData(void* data, size_t size, utility::ErrorState& error)
	{
		return setDataInternal(data, size, static_cast<VkBufferUsageFlagBits>(getBufferUsage(EBufferObjectType::Storage)), error);
	}
}
