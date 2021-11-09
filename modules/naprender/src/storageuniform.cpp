/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "storageuniform.h"

RTTI_DEFINE_BASE(nap::StorageUniformBlock)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageUniform)
	RTTI_PROPERTY("Name", &nap::StorageUniform::mName, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageUniformBlockBuffer)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageUniformValueBuffer)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageUniformStruct)
	RTTI_PROPERTY("UniformBlock", &nap::StorageUniformStruct::mUniformBlock, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


namespace nap
{

}
