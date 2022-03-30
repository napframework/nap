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

//////////////////////////////////////////////////////////////////////////
// GPUBuffer
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBuffer)
	RTTI_PROPERTY("Count", &nap::GPUBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// GPUBufferNumeric
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBufferUInt)
	RTTI_PROPERTY("FillPolicy", &nap::GPUBufferUInt::mFillPolicy, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBufferInt)
	RTTI_PROPERTY("FillPolicy", &nap::GPUBufferInt::mFillPolicy, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBufferFloat)
	RTTI_PROPERTY("FillPolicy", &nap::GPUBufferFloat::mFillPolicy, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBufferVec2)
	RTTI_PROPERTY("FillPolicy", &nap::GPUBufferVec2::mFillPolicy, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBufferVec3)
	RTTI_PROPERTY("FillPolicy", &nap::GPUBufferVec3::mFillPolicy, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBufferVec4)
	RTTI_PROPERTY("FillPolicy", &nap::GPUBufferVec4::mFillPolicy, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUBufferMat4)
	RTTI_PROPERTY("FillPolicy", &nap::GPUBufferMat4::mFillPolicy, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// VertexBuffer
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VertexBufferUInt)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Storage", &nap::VertexBufferUInt::mStorage, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VertexBufferInt)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Storage", &nap::VertexBufferInt::mStorage, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VertexBufferFloat)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Storage", &nap::VertexBufferFloat::mStorage, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VertexBufferVec2)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Storage", &nap::VertexBufferVec2::mStorage, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VertexBufferVec3)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Storage", &nap::VertexBufferVec3::mStorage, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VertexBufferVec4)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Storage", &nap::VertexBufferVec4::mStorage, nap::rtti::EPropertyMetaData::Default)
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
		ensureUsage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		return GPUBufferNumeric<uint>::init(errorState);
	}
}
