// Local Includes
#include "renderattributes.h"

// GLM attribute definitions
RTTI_DEFINE_DATA(glm::mat4x4)
RTTI_DEFINE_DATA(glm::mat3x3)
RTTI_DEFINE_DATA(glm::mat2x2)
RTTI_DEFINE_DATA(glm::vec2)
RTTI_DEFINE_DATA(glm::vec4)
RTTI_DEFINE_DATA(glm::vec3)
RTTI_DEFINE_DATA(glm::ivec2)
RTTI_DEFINE_DATA(glm::ivec3)
RTTI_DEFINE_DATA(glm::quat)

// TODO: move glm RTTI definitions to some central place

RTTI_BEGIN_CLASS(glm::vec2)
	RTTI_PROPERTY("x", &glm::vec2::x, RTTI::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::vec2::y, RTTI::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(glm::vec3)
	RTTI_PROPERTY("x", &glm::vec3::x, RTTI::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::vec3::y, RTTI::EPropertyMetaData::Default)
	RTTI_PROPERTY("z", &glm::vec3::z, RTTI::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(glm::vec4)
	RTTI_PROPERTY("x", &glm::vec4::x, RTTI::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::vec4::y, RTTI::EPropertyMetaData::Default)
	RTTI_PROPERTY("z", &glm::vec4::z, RTTI::EPropertyMetaData::Default)
	RTTI_PROPERTY("w", &glm::vec4::w, RTTI::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(glm::ivec2)
	RTTI_PROPERTY("x", &glm::ivec2::x, RTTI::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::ivec2::y, RTTI::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(glm::ivec3)
	RTTI_PROPERTY("x", &glm::ivec3::x, RTTI::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::ivec3::y, RTTI::EPropertyMetaData::Default)
	RTTI_PROPERTY("z", &glm::ivec3::z, RTTI::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(glm::quat)
	RTTI_PROPERTY("x", &glm::quat::x, RTTI::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::quat::y, RTTI::EPropertyMetaData::Default)
	RTTI_PROPERTY("z", &glm::quat::z, RTTI::EPropertyMetaData::Default)
	RTTI_PROPERTY("w", &glm::quat::w, RTTI::EPropertyMetaData::Default)
RTTI_END_CLASS