/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "gpubuffer.h"
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

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBufferUInt)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBufferInt)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBufferFloat)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBufferVec2)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBufferVec3)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBufferVec4)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBufferMat4)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// GPUBuffer
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBufferUInt)
	RTTI_PROPERTY("Count", &nap::GPUBufferUInt::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::GPUBufferUInt::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::GPUBufferUInt::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::GPUBufferUInt::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::GPUBufferUInt::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBufferInt)
	RTTI_PROPERTY("Count", &nap::GPUBufferInt::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::GPUBufferInt::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::GPUBufferInt::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::GPUBufferInt::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::GPUBufferInt::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBufferFloat)
	RTTI_PROPERTY("Count", &nap::GPUBufferFloat::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::GPUBufferFloat::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::GPUBufferFloat::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::GPUBufferFloat::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::GPUBufferFloat::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBufferVec2)
	RTTI_PROPERTY("Count", &nap::GPUBufferVec2::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::GPUBufferVec2::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::GPUBufferVec2::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::GPUBufferVec2::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::GPUBufferVec2::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBufferVec3)
	RTTI_PROPERTY("Count", &nap::GPUBufferVec3::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::GPUBufferVec3::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::GPUBufferVec3::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::GPUBufferVec3::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::GPUBufferVec3::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBufferVec4)
	RTTI_PROPERTY("Count", &nap::GPUBufferVec4::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::GPUBufferVec4::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::GPUBufferVec4::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::GPUBufferVec4::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::GPUBufferVec4::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBufferMat4)
	RTTI_PROPERTY("Count", &nap::GPUBufferMat4::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::GPUBufferMat4::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DescriptorType", &nap::GPUBufferMat4::mDescriptorType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::GPUBufferMat4::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clear", &nap::GPUBufferMat4::mClear, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// VertexBuffer
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VertexBufferUInt)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VertexBufferInt)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VertexBufferFloat)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VertexBufferVec2)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VertexBufferVec3)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VertexBufferVec4)
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
		GPUBufferNumeric<uint>::mUsageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		return GPUBufferNumeric<uint>::init(errorState);
	}
}
