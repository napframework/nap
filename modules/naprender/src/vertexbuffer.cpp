/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "vertexbuffer.h"
#include "renderservice.h"

// External Includes
#include "vulkan/vulkan.h"
#include <nap/core.h>
#include <assert.h>

RTTI_BEGIN_ENUM(nap::EVertexBufferFormat)
	RTTI_ENUM_VALUE(nap::EVertexBufferFormat::R8_SINT,				"R8_SINT"),
	RTTI_ENUM_VALUE(nap::EVertexBufferFormat::R32_SINT,				"R32_SINT"),
	RTTI_ENUM_VALUE(nap::EVertexBufferFormat::R32_SFLOAT,			"R32_SFLOAT"),
	RTTI_ENUM_VALUE(nap::EVertexBufferFormat::R64_SFLOAT,			"R64_SFLOAT"),
	RTTI_ENUM_VALUE(nap::EVertexBufferFormat::R32G32_SFLOAT,		"R32G32_SFLOAT"),
	RTTI_ENUM_VALUE(nap::EVertexBufferFormat::R32G32B32_SFLOAT,		"R32G32B32_SFLOAT"),
	RTTI_ENUM_VALUE(nap::EVertexBufferFormat::R32G32B32A32_SFLOAT,	"R32G32B32A32_SFLOAT")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VertexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Usage", &nap::VertexBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Format", &nap::VertexBuffer::mBufferFormat, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Utility functions
	//////////////////////////////////////////////////////////////////////////

	VkFormat getVulkanFormat(EVertexBufferFormat format)
	{
		static std::unordered_map<EVertexBufferFormat, VkFormat> format_map =
		{
			{ EVertexBufferFormat::R8_SINT,				VK_FORMAT_R8_SINT },
			{ EVertexBufferFormat::R32_SINT,			VK_FORMAT_R32_SINT },
			{ EVertexBufferFormat::R32_SFLOAT,			VK_FORMAT_R32_SFLOAT },
			{ EVertexBufferFormat::R64_SFLOAT,			VK_FORMAT_R64_SFLOAT },
			{ EVertexBufferFormat::R32G32_SFLOAT,		VK_FORMAT_R32G32_SFLOAT },
			{ EVertexBufferFormat::R32G32B32_SFLOAT,	VK_FORMAT_R32G32B32_SFLOAT },
			{ EVertexBufferFormat::R32G32B32A32_SFLOAT,	VK_FORMAT_R32G32B32A32_SFLOAT },
		};

		const auto it = format_map.find(format);
		if (it != format_map.end())
			return it->second;

		assert(false);
		return VK_FORMAT_UNDEFINED;
	}


	int getVertexElementSize(VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_R8_SINT:
			return 1;
		case VK_FORMAT_R32_SINT:
		case VK_FORMAT_R32_SFLOAT:
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


	//////////////////////////////////////////////////////////////////////////
	// VertexBuffer
	//////////////////////////////////////////////////////////////////////////

	VertexBuffer::VertexBuffer(Core& core) :
		GPUBuffer(core)
	{ }


	VertexBuffer::VertexBuffer(Core& core, VkFormat format, EMeshDataUsage usage) :
		GPUBuffer(core, usage), mFormat(format), mBufferFormat(EVertexBufferFormat::UNKNOWN)
	{ }


	bool VertexBuffer::init(utility::ErrorState& errorState)
	{
		// Set vulkan format if the vertex buffer format was set in the resource
		if (mBufferFormat != EVertexBufferFormat::UNKNOWN)
		{
			mFormat = getVulkanFormat(mBufferFormat);
			if (!errorState.check(mFormat != VK_FORMAT_UNDEFINED, "Undefined vulkan format"))
				return false;
		}

		mVertexSize = getVertexElementSize(mFormat);
		if (!errorState.check(mVertexSize > 0, "Unsupported Vulkan Format"))
			return false;

		return GPUBuffer::init(errorState);
	}


	// Uploads the data block to the GPU
	bool VertexBuffer::setData(void* data, size_t numVertices, size_t reservedNumVertices, utility::ErrorState& error)
	{
		return setDataInternal(data, mVertexSize, numVertices, reservedNumVertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, error);
	}
}
