/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include <rtti/rtti.h>
#include <rtti/path.h>
#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>

RTTI_BEGIN_STRUCT(glm::vec2)
	RTTI_VALUE_CONSTRUCTOR(float, float)
	RTTI_PROPERTY("x", &glm::vec2::x, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::vec2::y, nap::rtti::EPropertyMetaData::Default)
	RTTI_CUSTOM_REGISTRATION_FUNCTION(sRegisterFloatVectorOperators)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(glm::vec3)
	RTTI_VALUE_CONSTRUCTOR(float, float, float)
	RTTI_PROPERTY("x", &glm::vec3::x, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::vec3::y, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("z", &glm::vec3::z, nap::rtti::EPropertyMetaData::Default)
	RTTI_CUSTOM_REGISTRATION_FUNCTION(sRegisterFloatVectorOperators)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(glm::vec4)
	RTTI_VALUE_CONSTRUCTOR(float, float, float, float)
	RTTI_PROPERTY("x", &glm::vec4::x, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::vec4::y, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("z", &glm::vec4::z, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("w", &glm::vec4::w, nap::rtti::EPropertyMetaData::Default)
	RTTI_CUSTOM_REGISTRATION_FUNCTION(sRegisterFloatVectorOperators)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(glm::ivec2)
	RTTI_VALUE_CONSTRUCTOR(int, int)
	RTTI_PROPERTY("x", &glm::ivec2::x, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::ivec2::y, nap::rtti::EPropertyMetaData::Default)
	RTTI_CUSTOM_REGISTRATION_FUNCTION(sRegisterIntVectorOperators)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(glm::ivec3)
	RTTI_VALUE_CONSTRUCTOR(int, int, int)
	RTTI_PROPERTY("x", &glm::ivec3::x, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::ivec3::y, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("z", &glm::ivec3::z, nap::rtti::EPropertyMetaData::Default)
	RTTI_CUSTOM_REGISTRATION_FUNCTION(sRegisterIntVectorOperators)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(glm::ivec4)
	RTTI_VALUE_CONSTRUCTOR(int, int, int, int)
	RTTI_PROPERTY("x", &glm::ivec4::x, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::ivec4::y, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("z", &glm::ivec4::z, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("w", &glm::ivec4::w, nap::rtti::EPropertyMetaData::Default)
	RTTI_CUSTOM_REGISTRATION_FUNCTION(sRegisterIntVectorOperators)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(glm::quat)
	RTTI_VALUE_CONSTRUCTOR(float, float, float, float)
	RTTI_PROPERTY("x", &glm::quat::x, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::quat::y, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("z", &glm::quat::z, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("w", &glm::quat::w, nap::rtti::EPropertyMetaData::Default)
	RTTI_CUSTOM_REGISTRATION_FUNCTION(sRegisterQuatOperators)
RTTI_END_STRUCT
