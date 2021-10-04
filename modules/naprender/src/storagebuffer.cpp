/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "storagebuffer.h"

// External Includes
#include <assert.h>

namespace nap
{
	int getElementSize(VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_R8_SINT:
			return 1;
		case VK_FORMAT_R32_SFLOAT:
		case VK_FORMAT_R32_SINT:
			return 4;
		case VK_FORMAT_R64_SFLOAT:
		case VK_FORMAT_R32G32_SFLOAT:
			return 8;
		case VK_FORMAT_R32G32B32_SFLOAT:
			return 12;
		case VK_FORMAT_R32G32B32A32_SFLOAT:
			return 16;
		default:
			assert(false);
		}
		return -1;
	}


	StorageBuffer::StorageBuffer(RenderService& renderService, VkFormat format, EMeshDataUsage usage) :
		GPUBuffer(renderService, usage),
		mFormat(format),
		mElementSize(getElementSize(format))
	{ }


	bool StorageBuffer::setData(void* data, size_t size, VkBufferUsageFlagBits bufferUsage, utility::ErrorState& error)
	{
		// The buffer size must be a multiple of the element size
		assert(size % mElementSize == 0);

		mSize = size;
		return setDataInternal(data, mElementSize, size, size, bufferUsage, error);
	}
}
