/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "indexbuffer.h"

// External Includes
#include <assert.h>

namespace nap
{
	IndexBuffer::IndexBuffer(RenderService& renderService, EMeshDataUsage usage) :
		GPUBuffer(renderService, usage)
	{ }


	bool IndexBuffer::setData(const std::vector<uint32>& indices, utility::ErrorState& error)
	{
		mCount = indices.size();
		return setDataInternal((void*)indices.data(), sizeof(nap::uint32), indices.size(), indices.capacity(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, error);
	}
}