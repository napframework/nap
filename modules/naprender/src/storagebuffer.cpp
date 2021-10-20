/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "storagebuffer.h"

// External Includes
#include "vulkan/vulkan.h"
#include <assert.h>

namespace nap
{
	StorageBuffer::StorageBuffer(RenderService& renderService, EBufferObjectType type, EMeshDataUsage usage) :
		GPUBuffer(renderService, usage),
		mType(type)
	{ }


	// Uploads the data block to the GPU
	bool StorageBuffer::setData(void* data, size_t size, utility::ErrorState& error)
	{
		return setDataInternal(data, size, static_cast<VkBufferUsageFlagBits>(getBufferUsage(mType)), error);
	}
}
