/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "vertexbuffer.h"
#include "renderservice.h"

// External Includes
#include <nap/core.h>

RTTI_DEFINE_BASE(nap::VertexBuffer)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IntVertexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::IntVertexBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::IntVertexBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::IntVertexBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("VertexShaderAccess", &nap::IntVertexBuffer::mVertexShaderAccess, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ComputeShaderAccess", &nap::IntVertexBuffer::mComputeShaderAccess, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::IntVertexBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FloatVertexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::FloatVertexBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::FloatVertexBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::FloatVertexBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("VertexShaderAccess", &nap::FloatVertexBuffer::mVertexShaderAccess, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ComputeShaderAccess", &nap::FloatVertexBuffer::mComputeShaderAccess, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::FloatVertexBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Vec2VertexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::Vec2VertexBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::Vec2VertexBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::Vec2VertexBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("VertexShaderAccess", &nap::Vec2VertexBuffer::mVertexShaderAccess, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ComputeShaderAccess", &nap::Vec2VertexBuffer::mComputeShaderAccess, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::Vec2VertexBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Vec3VertexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::Vec3VertexBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::Vec3VertexBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::Vec3VertexBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("VertexShaderAccess", &nap::Vec3VertexBuffer::mVertexShaderAccess, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ComputeShaderAccess", &nap::Vec3VertexBuffer::mComputeShaderAccess, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::Vec3VertexBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Vec4VertexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::Vec4VertexBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::Vec4VertexBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::Vec4VertexBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("VertexShaderAccess", &nap::Vec4VertexBuffer::mVertexShaderAccess, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ComputeShaderAccess", &nap::Vec4VertexBuffer::mComputeShaderAccess, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::Vec4VertexBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// Mat4VertexBuffer (?)
