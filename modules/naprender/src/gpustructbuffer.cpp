/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "gpustructbuffer.h"
#include "renderservice.h"
#include "uniformutils.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUStructBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Descriptor", &nap::GPUStructBuffer::mDescriptor, nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Usage", &nap::GPUStructBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::GPUStructBuffer::mFillPolicy, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


namespace nap
{
	bool GPUStructBuffer::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mDescriptor.mCount >= 0, "Descriptor.Count must be non-zero and non-negative"))
			return false;

		if (!GPUBuffer::init(errorState))
			return false;

		UniformStruct* element_descriptor = mDescriptor.mElement.get();

		// Verify maximum depth
		int depth = getShaderVariableStructDepth(*element_descriptor);
		if (!errorState.check(depth == 0, "GPUStructBuffers with elements that exceed depth=1 are currently not supported"))
			return false;

		// Calculate element size in bytes
		mElementSize = getShaderVariableStructSizeRecursive(*element_descriptor);

		size_t buffer_size = getSize();
		VkBufferUsageFlagBits buffer_usage = static_cast<VkBufferUsageFlagBits>(getBufferUsage(EBufferObjectType::Storage));

		// If usage is DynamicRead, skip buffer fill and upload
		if (mUsage == EMeshDataUsage::DynamicRead)
			return allocateInternal(buffer_size, buffer_usage, errorState);

		// Create a staging buffer to upload
		auto staging_buffer = std::make_unique<uint8[]>(buffer_size);
		if (mFillPolicy != nullptr)
		{
			mFillPolicy->fill(&mDescriptor, staging_buffer.get(), errorState);
		}
		else
		{
			std::memset(staging_buffer.get(), 0, buffer_size);
		}

		// Prepare staging buffer upload
		return setDataInternal(staging_buffer.get(), buffer_size, buffer_size, buffer_usage, errorState);
	}


	bool GPUStructBuffer::setData(void* data, size_t size, utility::ErrorState& error)
	{
		VkBufferUsageFlagBits buffer_usage = static_cast<VkBufferUsageFlagBits>(getBufferUsage(EBufferObjectType::Storage));
		return setDataInternal(data, size, size, buffer_usage, error);
	}
}
