/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "vertexattribute.h"


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BaseVertexAttribute)
	RTTI_PROPERTY("AttributeID", &nap::BaseVertexAttribute::mAttributeID, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::FloatVertexAttribute)
	RTTI_PROPERTY("Data", &nap::FloatVertexAttribute::mData, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::IntVertexAttribute)
	RTTI_PROPERTY("Data", &nap::IntVertexAttribute::mData, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ByteVertexAttribute)
	RTTI_PROPERTY("Data", &nap::ByteVertexAttribute::mData, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::DoubleVertexAttribute)
	RTTI_PROPERTY("Data", &nap::DoubleVertexAttribute::mData, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Vec2VertexAttribute)
	RTTI_PROPERTY("Data", &nap::Vec2VertexAttribute::mData, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Vec3VertexAttribute)
	RTTI_PROPERTY("Data", &nap::Vec3VertexAttribute::mData, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Vec4VertexAttribute)
	RTTI_PROPERTY("Data", &nap::Vec4VertexAttribute::mData, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	// Only here to make sure this cpp is not removed during optimization, which would cause the RTTI definitions to be missing
	BaseVertexAttribute::BaseVertexAttribute()
	{
	}

	//////////////////////////////////////////////////////////////////////////

	template<>
	VkFormat FloatVertexAttribute::getFormat() const { return VK_FORMAT_R32_SFLOAT; }

	template<>
	VkFormat IntVertexAttribute::getFormat() const { return VK_FORMAT_R32_SINT; }

	template<>
	VkFormat ByteVertexAttribute::getFormat() const { return VK_FORMAT_R8_SINT; }

	template<>
	VkFormat DoubleVertexAttribute::getFormat() const { return VK_FORMAT_R64_SFLOAT; }

	template<>
	VkFormat Vec2VertexAttribute::getFormat() const { return VK_FORMAT_R32G32_SFLOAT; }

	template<>
	VkFormat Vec3VertexAttribute::getFormat() const { return VK_FORMAT_R32G32B32_SFLOAT; }

	template<>
	VkFormat Vec4VertexAttribute::getFormat() const { return VK_FORMAT_R32G32B32A32_SFLOAT; }
}