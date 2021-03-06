/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "vertexbuffer.h"

// External Includes
#include "vulkan/vulkan.h"
#include <assert.h>

namespace nap
{
	int getVertexSize(VkFormat format)
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


	VertexBuffer::VertexBuffer(RenderService& renderService, VkFormat inFormat, EMeshDataUsage inUsage) :
		GPUBuffer(renderService, inUsage),
		mFormat(inFormat),
		mVertexSize(getVertexSize(inFormat))
	{ }


	// Uploads the data block to the GPU
	bool VertexBuffer::setData(void* data, size_t numVertices, size_t reservedNumVertices, utility::ErrorState& error)
	{
		return setDataInternal(data, mVertexSize, numVertices, reservedNumVertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,  error);
	}

} // opengl