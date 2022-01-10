/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "indexbuffer.h"
#include "renderservice.h"

// External Includes
#include <nap/core.h>
#include <assert.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IndexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::IndexBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::IndexBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::IndexBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::IndexBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	bool IndexBuffer::init(utility::ErrorState& errorState)
	{
		mUsageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		if (!IntVertexBuffer::init(errorState))
		{
			errorState.fail("Failed to initialize index buffer");
			return false;
		}
		return true;
	}


	bool IndexBuffer::setData(const std::vector<uint32>& indices, utility::ErrorState& error)
	{
		return setDataInternal((void*)indices.data(), sizeof(nap::uint32), indices.size(), indices.capacity(), mUsageFlags, error);
	}
}
