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
	RTTI_PROPERTY("Usage", &nap::IndexBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// IndexBuffer
	//////////////////////////////////////////////////////////////////////////

	IndexBuffer::IndexBuffer(Core& core) :
		GPUBuffer(core)
	{ }


	IndexBuffer::IndexBuffer(Core& core, EMeshDataUsage usage) :
		GPUBuffer(core, usage)
	{ }


	bool IndexBuffer::init(utility::ErrorState& errorState)
	{
		return GPUBuffer::init(errorState);
	}

	bool IndexBuffer::setData(const std::vector<uint32>& indices, utility::ErrorState& error)
	{
		mCount = indices.size();
		return setDataInternal((void*)indices.data(), sizeof(nap::uint32), indices.size(), indices.capacity(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, error);
	}
}
