/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "valuegpubuffer.h"
#include "renderservice.h"
#include "mathutils.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBuffer)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////
// TypedValueGPUBuffer
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UIntGPUBuffer)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IntGPUBuffer)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FloatGPUBuffer)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Vec2GPUBuffer)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Vec3GPUBuffer)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Vec4GPUBuffer)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Mat4GPUBuffer)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// GPUBuffer
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UIntGPUBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::UIntGPUBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::UIntGPUBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::UIntGPUBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::UIntGPUBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::UIntGPUBuffer::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IntGPUBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::IntGPUBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::IntGPUBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::IntGPUBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::IntGPUBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::IntGPUBuffer::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FloatGPUBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::FloatGPUBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::FloatGPUBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::FloatGPUBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::FloatGPUBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::FloatGPUBuffer::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Vec2GPUBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::Vec2GPUBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::Vec2GPUBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::Vec2GPUBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::Vec2GPUBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::Vec2GPUBuffer::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Vec3GPUBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::Vec3GPUBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::Vec3GPUBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::Vec3GPUBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::Vec3GPUBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::Vec3GPUBuffer::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Vec4GPUBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::Vec4GPUBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::Vec4GPUBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::Vec4GPUBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::Vec4GPUBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::Vec4GPUBuffer::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Mat4GPUBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::Mat4GPUBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::Mat4GPUBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::Mat4GPUBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::Mat4GPUBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::Mat4GPUBuffer::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// VertexBuffer
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UIntVertexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IntVertexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FloatVertexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Vec2VertexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Vec3VertexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Vec4VertexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// IndexBuffer
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IndexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS


namespace nap
{
	bool IndexBuffer::init(utility::ErrorState& errorState)
	{
		GPUBufferNumeric<uint>::mUsageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		return GPUBufferNumeric<uint>::init(errorState);
	}
}
