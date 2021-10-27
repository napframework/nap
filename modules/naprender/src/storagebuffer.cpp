/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "storagebuffer.h"
#include "renderservice.h"

// External Includes
#include "vulkan/vulkan.h"
#include <nap/core.h>
#include <assert.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageIntBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::StorageIntBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::StorageIntBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BufferObjectType", &nap::StorageIntBuffer::mType, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageFloatBuffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::StorageFloatBuffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::StorageFloatBuffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BufferObjectType", &nap::StorageFloatBuffer::mType, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageVec2Buffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::StorageVec2Buffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::StorageVec2Buffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BufferObjectType", &nap::StorageVec2Buffer::mType, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageVec3Buffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::StorageVec3Buffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::StorageVec3Buffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BufferObjectType", &nap::StorageVec3Buffer::mType, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageVec4Buffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::StorageVec4Buffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::StorageVec4Buffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BufferObjectType", &nap::StorageVec4Buffer::mType, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageMat4Buffer)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Count", &nap::StorageMat4Buffer::mCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage", &nap::StorageMat4Buffer::mUsage, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BufferObjectType", &nap::StorageMat4Buffer::mType, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS
