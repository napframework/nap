/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "structbuffer.h"
#include "renderservice.h"
#include "uniformutils.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StructBuffer, "GPU data buffer that stores nested data structures")
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Clear",		&nap::StructBuffer::mClear,			nap::rtti::EPropertyMetaData::Default,	"Initialize to zero without a fill policy")
	RTTI_PROPERTY("Descriptor", &nap::StructBuffer::mDescriptor,	nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::Embedded, "The descriptor that defines the layout")
	RTTI_PROPERTY("FillPolicy", &nap::StructBuffer::mFillPolicy,	nap::rtti::EPropertyMetaData::Default, "Optional rule that defines how to fill the buffer")
RTTI_END_CLASS

namespace nap
{
	bool StructBuffer::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mDescriptor.mCount >= 0, "Struct buffer descriptor's 'Count' property must be non-zero and non-negative"))
			return false;

		// Ensure buffer can be tied to a shader descriptor set slot
		ensureUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		if (!GPUBuffer::init(errorState))
			return false;

		// Calculate element size in bytes
		mElementSize = getUniformStructSizeRecursive(*mDescriptor.mElement);
		size_t buffer_size = getSize();

		// Allocate buffer memory
		if (!allocateInternal(buffer_size, errorState))
			return false;

		// Upload data when a buffer fill policy is available
		if (mFillPolicy != nullptr)
		{
			if (mMemoryUsage != EMemoryUsage::DynamicRead)
			{
				// Create a staging buffer to upload
				auto staging_buffer = std::make_unique<uint8[]>(buffer_size);
				if (!mFillPolicy->fill(&mDescriptor, staging_buffer.get(), errorState))
					return false;

				// Prepare staging buffer upload
				if (!setDataInternal(staging_buffer.get(), buffer_size, buffer_size, errorState))
					return false;
			}
			else
			{
				// Warn user that buffers cannot be filled when their usage is set to DynamicRead
				nap::Logger::warn(utility::stringFormat("%s: The configured fill policy was ignored as the buffer usage is DynamicRead", mID.c_str()).c_str());
			}
		}

		// Optionally clear - does not count as an upload
		else if (mClear)
		{
			GPUBuffer::requestClear();
		}

		mInitialized = true;
		return true;
	}


	bool StructBuffer::setData(void* data, size_t size, utility::ErrorState& error)
	{
		return setDataInternal(data, size, size, error);
	}
}
