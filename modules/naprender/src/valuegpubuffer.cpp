/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "valuegpubuffer.h"
#include "renderservice.h"

// External Includes
#include <nap/core.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ValueGPUBuffer)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////
// TypedValueGPUBuffer
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TypedValueGPUBuffer<nap::uint>)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TypedValueGPUBuffer<int>)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TypedValueGPUBuffer<float>)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TypedValueGPUBuffer<glm::vec2> )
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TypedValueGPUBuffer<glm::vec3>)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TypedValueGPUBuffer<glm::vec4>)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TypedValueGPUBuffer<glm::mat4>)
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
	RTTI_PROPERTY("Count", &nap::UIntVertexBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::UIntVertexBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::UIntVertexBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::UIntVertexBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::UIntVertexBuffer::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IntVertexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::IntVertexBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::IntVertexBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::IntVertexBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::IntVertexBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::IntVertexBuffer::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FloatVertexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::FloatVertexBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::FloatVertexBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::FloatVertexBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::FloatVertexBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::FloatVertexBuffer::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Vec2VertexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::Vec2VertexBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::Vec2VertexBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::Vec2VertexBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::Vec2VertexBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::Vec2VertexBuffer::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Vec3VertexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::Vec3VertexBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::Vec3VertexBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::Vec3VertexBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::Vec3VertexBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::Vec3VertexBuffer::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Vec4VertexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::Vec4VertexBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::Vec4VertexBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::Vec4VertexBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::Vec4VertexBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::Vec4VertexBuffer::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// IndexBuffer
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::IndexBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::IndexBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::IndexBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::IndexBuffer::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::IndexBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::IndexBuffer::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS
