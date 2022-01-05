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
	RTTI_ENUM_VALUE(nap::EVertexBufferFormat::Byte,		"Byte"),
	RTTI_ENUM_VALUE(nap::EVertexBufferFormat::Int,		"Int"),
	RTTI_ENUM_VALUE(nap::EVertexBufferFormat::Float,	"Float"),
	RTTI_ENUM_VALUE(nap::EVertexBufferFormat::Double,	"Double"),
	RTTI_ENUM_VALUE(nap::EVertexBufferFormat::Vec2,		"Vec2"),
	RTTI_ENUM_VALUE(nap::EVertexBufferFormat::Vec3,		"Vec3"),
	RTTI_ENUM_VALUE(nap::EVertexBufferFormat::Vec4,		"Vec4"),
	RTTI_ENUM_VALUE(nap::EVertexBufferFormat::Unknown,	"Unknown")
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

	/**
	 * Returns the vulkan format for the given vertex buffer format
	 * @param format requested format
	 * @return vulkan format
	 */
	VkFormat getVulkanFormat(EVertexBufferFormat format)
	{
		static std::unordered_map<EVertexBufferFormat, VkFormat> format_map =
		{
			{ EVertexBufferFormat::Byte,	VK_FORMAT_R8_SINT },
			{ EVertexBufferFormat::Int,		VK_FORMAT_R32_SINT },
			{ EVertexBufferFormat::Float,	VK_FORMAT_R32_SFLOAT },
			{ EVertexBufferFormat::Double,	VK_FORMAT_R64_SFLOAT },
			{ EVertexBufferFormat::Vec2,	VK_FORMAT_R32G32_SFLOAT },
			{ EVertexBufferFormat::Vec3,	VK_FORMAT_R32G32B32_SFLOAT },
			{ EVertexBufferFormat::Vec4,	VK_FORMAT_R32G32B32A32_SFLOAT },
		};

		const auto it = format_map.find(format);
		if (it != format_map.end())
			return it->second;

		assert(false);
		return VK_FORMAT_UNDEFINED;
	}

	/**
	 * Returns the size in bytes, for a single element, of the given format.
	 * @param format requested format
	 * @return size in bytes, for a single element, of the given format. -1 if unsupported.
	 */
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
		GPUBuffer(core, usage), mFormat(format), mBufferFormat(EVertexBufferFormat::Unknown)
	{ }


	bool VertexBuffer::init(utility::ErrorState& errorState)
	{
		// Set vulkan format if the vertex buffer was constructed in json
		if (mBufferFormat != EVertexBufferFormat::Unknown)
		{
			mFormat = getVulkanFormat(mBufferFormat);
			if (!errorState.check(mFormat != VK_FORMAT_UNDEFINED, "Undefined vulkan format"))
				return false;
		}

		if (!errorState.check(getVertexElementSize(mFormat) > 0, "Unsupported Vulkan Format"))
			return false;

		return GPUBuffer::init(errorState);
	}


	// Uploads the data block to the GPU
	bool VertexBuffer::setData(void* data, size_t numVertices, size_t reservedNumVertices, utility::ErrorState& error)
	{
		return setDataInternal(data, getVertexElementSize(mFormat), numVertices, reservedNumVertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, error);
	}
}
