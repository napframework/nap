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
	RTTI_PROPERTY("DescriptorType", &nap::GPUStructBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
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

		// Compose usage flags from buffer configuration
		mUsageFlags = getBufferUsage(mDescriptorType);

		// Allocate buffer memory
		if (!allocateInternal(buffer_size, mUsageFlags, errorState))
			return false;

		// Upload data when a buffer fill policy is available
		if (mFillPolicy != nullptr)
		{
			if (mUsage != EMeshDataUsage::DynamicRead)
			{
				// Create a staging buffer to upload
				auto staging_buffer = std::make_unique<uint8[]>(buffer_size);

				if (!mFillPolicy->fill(&mDescriptor, staging_buffer.get(), errorState))
					return false;

				// Prepare staging buffer upload
				if (!setDataInternal(staging_buffer.get(), buffer_size, buffer_size, 0, errorState))
					return false;
			}
			else
			{
				// Warn user that buffers cannot be filled when their usage is set to DynamicRead
				nap::Logger::warn(utility::stringFormat("%s: The configured fill policy was ignored as the buffer usage is DynamicRead", mID.c_str()).c_str());
			}
		}
		else
		{
			// TODO: Implement optional Clear
			//std::memset(staging_buffer.get(), 0, buffer_size);
		}

		mInitialized = true;
		return true;
	}


	bool GPUStructBuffer::setData(void* data, size_t size, utility::ErrorState& error)
	{
		return setDataInternal(data, size, size, mUsageFlags, error);
	}
}
