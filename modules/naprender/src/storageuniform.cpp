/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "storageuniform.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageUniform)
	RTTI_PROPERTY("Name", &nap::StorageUniform::mName, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageUniformBuffer)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageUniformValueBuffer)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::StorageUniformStruct)
	RTTI_PROPERTY("StorageUniform", &nap::StorageUniformStruct::mStorageUniformBuffer, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::StorageUniformStructBuffer)
	RTTI_PROPERTY("Buffer", &nap::StorageUniformStructBuffer::mBuffer, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::StorageUniformIntBuffer)
	RTTI_PROPERTY("Buffer", &nap::StorageUniformIntBuffer::mBuffer, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::StorageUniformFloatBuffer)
	RTTI_PROPERTY("Buffer", &nap::StorageUniformFloatBuffer::mBuffer, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::StorageUniformVec2Buffer)
	RTTI_PROPERTY("Buffer", &nap::StorageUniformVec2Buffer::mBuffer, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::StorageUniformVec3Buffer)
	RTTI_PROPERTY("Buffer", &nap::StorageUniformVec3Buffer::mBuffer, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::StorageUniformVec4Buffer)
	RTTI_PROPERTY("Buffer", &nap::StorageUniformVec4Buffer::mBuffer, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::StorageUniformMat4Buffer)
	RTTI_PROPERTY("Buffer", &nap::StorageUniformMat4Buffer::mBuffer, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS
