/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "gpuvaluebuffer.h"
#include "renderservice.h"

// External Includes
#include <nap/core.h>

RTTI_DEFINE_BASE(nap::GPUValueBuffer)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUIntBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::GPUIntBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::GPUIntBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BufferObjectType", &nap::GPUIntBuffer::mType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::GPUIntBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUFloatBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::GPUFloatBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::GPUFloatBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BufferObjectType", &nap::GPUFloatBuffer::mType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::GPUFloatBuffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUVec2Buffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::GPUVec2Buffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::GPUVec2Buffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BufferObjectType", &nap::GPUVec2Buffer::mType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::GPUVec2Buffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUVec3Buffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::GPUVec3Buffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::GPUVec3Buffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BufferObjectType", &nap::GPUVec3Buffer::mType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::GPUVec3Buffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUVec4Buffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::GPUVec4Buffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::GPUVec4Buffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BufferObjectType", &nap::GPUVec4Buffer::mType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::GPUVec4Buffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GPUMat4Buffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::GPUMat4Buffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::GPUMat4Buffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BufferObjectType", &nap::GPUMat4Buffer::mType, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FillPolicy", &nap::GPUMat4Buffer::mBufferFillPolicy, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS
