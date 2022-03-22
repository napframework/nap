/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "storageuniform.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BufferBinding)
	RTTI_PROPERTY("Name", &nap::BufferBinding::mName, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BufferBindingNumeric)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::BufferBindingStruct)
	RTTI_PROPERTY("Buffer", &nap::BufferBindingStruct::mBuffer, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::BufferBindingUInt)
	RTTI_PROPERTY("Buffer", &nap::BufferBindingUInt::mBuffer, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::BufferBindingInt)
	RTTI_PROPERTY("Buffer", &nap::BufferBindingInt::mBuffer, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::BufferBindingFloat)
	RTTI_PROPERTY("Buffer", &nap::BufferBindingFloat::mBuffer, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::BufferBindingVec2)
	RTTI_PROPERTY("Buffer", &nap::BufferBindingVec2::mBuffer, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::BufferBindingVec3)
	RTTI_PROPERTY("Buffer", &nap::BufferBindingVec3::mBuffer, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::BufferBindingVec4)
	RTTI_PROPERTY("Buffer", &nap::BufferBindingVec4::mBuffer, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::BufferBindingMat4)
	RTTI_PROPERTY("Buffer", &nap::BufferBindingMat4::mBuffer, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS
