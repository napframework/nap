/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "gpudatabuffer.h"
#include "renderservice.h"

// External Includes
#include "vulkan/vulkan.h"
#include <nap/core.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUDataBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Usage", &nap::GPUDataBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Type", &nap::GPUDataBuffer::mType, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	GPUDataBuffer::GPUDataBuffer(nap::Core& core) :
		GPUBuffer(core)
	{ }


	GPUDataBuffer::GPUDataBuffer(Core& core, EBufferObjectType type, EMeshDataUsage usage) :
		GPUBuffer(core, usage), mType(type)
	{ }


	bool GPUDataBuffer::init(utility::ErrorState& errorState)
	{
		return GPUBuffer::init(errorState);
	}


	bool GPUDataBuffer::setData(void* data, size_t size, utility::ErrorState& error)
	{
		return setDataInternal(data, size, static_cast<VkBufferUsageFlagBits>(getBufferUsage(mType)), error);
	}
}
