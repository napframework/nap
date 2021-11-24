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
	RTTI_PROPERTY("FillPolicy", &nap::GPUStructBuffer::mFillPolicy, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
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
		size_t total_size = getSize();

		// Create a staging buffer to upload
		auto staging_buffer = std::make_unique<uint8[]>(total_size);
		if (mFillPolicy != nullptr)
		{
			mFillPolicy->fill(&mDescriptor, staging_buffer.get(), errorState);
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
