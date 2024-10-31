/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "bufferbinding.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BufferBinding, "Large shader buffer binding")
	RTTI_PROPERTY("Name", &nap::BufferBinding::mName, nap::rtti::EPropertyMetaData::Default, "Buffer binding name in shader")
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BufferBindingNumeric)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::BufferBindingStruct, "Struct shader buffer binding")
	RTTI_PROPERTY("Buffer", &nap::BufferBindingStruct::mBuffer, nap::rtti::EPropertyMetaData::Required, "Struct data to bind")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::BufferBindingUInt, "Numeric (uint) shader buffer binding")
	RTTI_PROPERTY("Buffer", &nap::BufferBindingUInt::mBuffer, nap::rtti::EPropertyMetaData::Required, "Unsigned integer data to bind")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::BufferBindingInt, "Numeric (int) shader buffer binding")
	RTTI_PROPERTY("Buffer", &nap::BufferBindingInt::mBuffer, nap::rtti::EPropertyMetaData::Required, "Integer data to bind")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::BufferBindingFloat, "Numeric (float) shader buffer binding")
	RTTI_PROPERTY("Buffer", &nap::BufferBindingFloat::mBuffer, nap::rtti::EPropertyMetaData::Required, "Float data to bind")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::BufferBindingVec2, "Vector (vec2) shader buffer binding")
	RTTI_PROPERTY("Buffer", &nap::BufferBindingVec2::mBuffer, nap::rtti::EPropertyMetaData::Required, "Vec2 data to bind")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::BufferBindingVec3, "Vector (vec3) shader buffer binding")
	RTTI_PROPERTY("Buffer", &nap::BufferBindingVec3::mBuffer, nap::rtti::EPropertyMetaData::Required, "Vec3 data to bind")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::BufferBindingVec4, "Vector (vec4) shader buffer binding")
	RTTI_PROPERTY("Buffer", &nap::BufferBindingVec4::mBuffer, nap::rtti::EPropertyMetaData::Required, "Vec4 data to bind")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::BufferBindingIVec4, "Vector (ivec4) shader buffer binding")
	RTTI_PROPERTY("Buffer", &nap::BufferBindingIVec4::mBuffer, nap::rtti::EPropertyMetaData::Required, "IVec4 data to bind")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::BufferBindingUVec4, "Vector (uvec4) shader buffer binding")
	RTTI_PROPERTY("Buffer", &nap::BufferBindingUVec4::mBuffer, nap::rtti::EPropertyMetaData::Required, "UVec4 data to bind")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::BufferBindingMat4, "Matrix (4x4) shader buffer binding")
	RTTI_PROPERTY("Buffer", &nap::BufferBindingMat4::mBuffer, nap::rtti::EPropertyMetaData::Required, "Mat4 data to bind")
RTTI_END_CLASS


bool nap::BufferBinding::init(utility::ErrorState& errorState)
{
	return errorState.check(mBuffer != nullptr, "Missing link to GPUBuffer");
}


bool nap::BufferBindingStruct::init(utility::ErrorState& errorState)
{
	BufferBinding::mBuffer = BufferBindingStruct::mBuffer.get();
	return BufferBinding::init(errorState);
}
